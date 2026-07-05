"""
知识库模块 - ChromaDB 封装
提供向量存储、语义检索、文档管理功能
"""

import uuid
import chromadb
from chromadb.config import Settings
from src.core.config import config
from src.core.llm_service import llm_service


class KnowledgeBase:
    """
    知识库封装
    使用 ChromaDB 作为向量存储后端，支持：
    - 文档块的添加与向量化
    - 语义相似度检索
    - 文档管理（列表、删除）
    """

    def __init__(self):
        self.client = chromadb.PersistentClient(
            path=config.CHROMA_PERSIST_DIR,
            settings=Settings(anonymized_telemetry=False),
        )
        self.collection_name = "course_materials"
        self._ensure_collection()

    def _ensure_collection(self):
        """确保集合存在"""
        try:
            self.collection = self.client.get_collection(self.collection_name)
        except Exception:
            self.collection = self.client.create_collection(
                name=self.collection_name,
                metadata={"description": "课件知识库 - 文档块与向量存储"},
            )

    def add_documents(
        self,
        chunks: list[str],
        metadatas: list[dict] = None,
        ids: list[str] = None,
    ) -> list[str]:
        """
        添加文档块到知识库
        Args:
            chunks: 文本块列表
            metadatas: 每个块的元数据列表（文件名、页码等）
            ids: 自定义ID列表（None则自动生成）
        Returns:
            添加的文档ID列表
        """
        if not chunks:
            return []

        # 生成向量嵌入
        embeddings = []
        for chunk in chunks:
            embedding = llm_service.generate_embedding(chunk)
            embeddings.append(embedding)

        # 生成ID
        if ids is None:
            ids = [str(uuid.uuid4()) for _ in chunks]

        # 如果没有元数据，生成默认
        if metadatas is None:
            metadatas = [{"source": "unknown"} for _ in chunks]

        # 写入ChromaDB
        self.collection.add(
            ids=ids,
            documents=chunks,
            embeddings=embeddings,
            metadatas=metadatas,
        )

        return ids

    def get_all_chunks(self, max_chars: int = 8000) -> str:
        """获取知识库所有文档块的合并文本（用于总结等需要全局视角的场景）"""
        try:
            results = self.collection.get()
            if not results["ids"]:
                return ""
            docs = results.get("documents", [])
            merged = ""
            for doc in docs:
                if len(merged) + len(doc) > max_chars:
                    remaining = max_chars - len(merged)
                    if remaining > 200:
                        merged += doc[:remaining] + "\n\n"
                    break
                merged += doc + "\n\n"
            return merged.strip()
        except Exception:
            return ""

    def search(self, query: str, top_k: int = None) -> list[dict]:
        """
        语义检索
        Args:
            query: 查询文本
            top_k: 返回Top-K结果
        Returns:
            [{"content": str, "metadata": dict, "score": float}, ...]
        """
        if top_k is None:
            top_k = config.RETRIEVAL_TOP_K

        query_embedding = llm_service.generate_embedding(query)

        results = self.collection.query(
            query_embeddings=[query_embedding],
            n_results=top_k,
        )

        # 格式化结果
        formatted = []
        if results["ids"] and results["ids"][0]:
            for i, doc_id in enumerate(results["ids"][0]):
                formatted.append({
                    "id": doc_id,
                    "content": results["documents"][0][i] if results["documents"] else "",
                    "metadata": results["metadatas"][0][i] if results["metadatas"] else {},
                    "score": 1 - (results["distances"][0][i] if results["distances"] else 0),
                })

        return formatted

    def get_all_documents(self) -> list[dict]:
        """获取知识库中所有文档的概览信息"""
        try:
            results = self.collection.get()
            if not results["ids"]:
                return []

            # 按源文件分组统计
            sources = {}
            for i, meta in enumerate(results["metadatas"]):
                source = meta.get("source", "unknown")
                if source not in sources:
                    sources[source] = {"filename": source, "chunk_count": 0, "ids": []}
                sources[source]["chunk_count"] += 1
                sources[source]["ids"].append(results["ids"][i])

            return list(sources.values())
        except Exception:
            return []

    def delete_document(self, filename: str) -> int:
        """删除指定文件的所有文档块"""
        try:
            results = self.collection.get(
                where={"source": filename}
            )
            if results["ids"]:
                self.collection.delete(ids=results["ids"])
                return len(results["ids"])
            return 0
        except Exception:
            return 0

    def get_chunks_by_file(self, filename: str) -> list[str]:
        """获取指定文件的全部文本块内容"""
        try:
            results = self.collection.get(
                where={"source": filename}
            )
            return results.get("documents", []) or []
        except Exception:
            return []

    def get_collection_stats(self) -> dict:
        """获取知识库统计信息"""
        try:
            count = self.collection.count()
            sources = self.get_all_documents()
            return {
                "total_chunks": count,
                "total_documents": len(sources),
                "documents": sources,
            }
        except Exception:
            return {"total_chunks": 0, "total_documents": 0, "documents": []}


# 全局知识库实例
knowledge_base = KnowledgeBase()
