"""
文档处理流水线 (管道-过滤器风格)
实现: 格式检测 → 文本提取 → 文本清洗 → 智能分块 → 向量嵌入 → 存入ChromaDB
"""

import re
import os
import uuid
from pathlib import Path
from typing import List, Tuple
from src.core.config import config
from src.core.knowledge_base import knowledge_base


class DocumentProcessor:
    """
    文档处理流水线
    采用管道-过滤器风格: 每个方法是一个独立的Filter,
    通过内存传递数据, 最终写入知识库
    """

    # 支持的文件格式
    SUPPORTED_FORMATS = {".pdf", ".md", ".txt", ".docx"}

    @staticmethod
    def detect_format(filename: str) -> Tuple[str, str]:
        """
        Filter 1: 格式检测
        根据文件扩展名识别文件类型
        """
        ext = Path(filename).suffix.lower()
        if ext not in DocumentProcessor.SUPPORTED_FORMATS:
            raise ValueError(f"不支持的文件格式: {ext}。支持的格式: {DocumentProcessor.SUPPORTED_FORMATS}")
        return ext, filename

    @staticmethod
    def extract_text(ext: str, file_bytes: bytes) -> str:
        """
        Filter 2: 文本提取
        根据文件类型调用对应的解析器
        """
        if ext == ".txt":
            return file_bytes.decode("utf-8", errors="ignore")

        elif ext == ".md":
            return file_bytes.decode("utf-8", errors="ignore")

        elif ext == ".pdf":
            # 使用 PyPDF2 提取PDF文本
            import io
            from PyPDF2 import PdfReader
            reader = PdfReader(io.BytesIO(file_bytes))
            text_parts = []
            for page in reader.pages:
                page_text = page.extract_text()
                if page_text:
                    text_parts.append(page_text)
            return "\n\n".join(text_parts)

        elif ext == ".docx":
            import io
            from docx import Document
            doc = Document(io.BytesIO(file_bytes))
            text_parts = []
            for para in doc.paragraphs:
                if para.text.strip():
                    text_parts.append(para.text)
            return "\n".join(text_parts)

        return ""

    @staticmethod
    def clean_text(raw_text: str) -> str:
        """
        Filter 3: 文本清洗
        去除乱码字符、多余空行、格式噪音
        """
        if not raw_text:
            return ""

        # 去除空字节
        text = raw_text.replace("\x00", "")

        # 统一换行符
        text = text.replace("\r\n", "\n").replace("\r", "\n")

        # 去除连续3个以上的空行
        text = re.sub(r'\n{4,}', '\n\n\n', text)

        # 去除行首行尾多余空格
        text = '\n'.join(line.strip() for line in text.split('\n'))

        # 去除不可打印字符（保留常用中英文及符号）
        allowed = r'a-zA-Z0-9一-鿿　-〿＀-￯\s.,;:!?()(){}[]、。《》·""''+-=*/\\@#$%^&|~`<>'
        text = re.sub(f'[^{allowed}]', '', text)

        # 去除连续空行
        text = re.sub(r'\n\s*\n\s*\n', '\n\n', text)

        return text.strip()

    @staticmethod
    def chunk_text(text: str, chunk_size: int = None, overlap: int = None) -> List[str]:
        """
        Filter 4: 智能分块
        按语义边界将文本切分为适合嵌入的块
        """
        if chunk_size is None:
            chunk_size = config.CHUNK_SIZE
        if overlap is None:
            overlap = config.CHUNK_OVERLAP

        if not text:
            return []

        chunks = []
        start = 0
        while start < len(text):
            end = start + chunk_size

            if end >= len(text):
                chunks.append(text[start:])
                break

            # 尝试在段落边界处截断
            chunk = text[start:end]
            # 找最近的段落分隔符
            last_break = max(
                chunk.rfind('\n\n'),
                chunk.rfind('\n'),
                chunk.rfind('。'),
                chunk.rfind('. '),
                chunk.rfind('；'),
            )

            if last_break > chunk_size // 2:
                end = start + last_break + 1
            # 否则按最后一个完整词截断
            else:
                last_space = chunk.rfind(' ')
                if last_space > chunk_size // 2:
                    end = start + last_space + 1

            chunks.append(text[start:end].strip())
            start = end - overlap  # 重叠部分

        return [c for c in chunks if len(c) > 50]  # 过滤过短的块

    @classmethod
    def process_file(cls, file_bytes: bytes, filename: str) -> dict:
        """
        完整的文档处理流水线
        串联所有Filter, 将处理结果写入知识库
        Returns:
            {"filename": str, "chunks_added": int, "chunk_ids": list}
        """
        # Filter 1: 格式检测
        ext, name = cls.detect_format(filename)

        # Filter 2: 文本提取
        raw_text = cls.extract_text(ext, file_bytes)

        if not raw_text.strip():
            return {"filename": filename, "chunks_added": 0, "error": "文件内容为空或无法提取文本"}

        # Filter 3: 文本清洗
        clean_text = cls.clean_text(raw_text)

        if not clean_text:
            return {"filename": filename, "chunks_added": 0, "error": "清洗后文本为空"}

        # Filter 4: 智能分块
        chunks = cls.chunk_text(clean_text)

        if not chunks:
            return {"filename": filename, "chunks_added": 0, "error": "分块结果为空"}

        # Filter 5+6: 向量嵌入 + 存入ChromaDB
        metadatas = [{"source": filename, "chunk_index": i, "total_chunks": len(chunks)}
                     for i in range(len(chunks))]
        chunk_ids = knowledge_base.add_documents(chunks, metadatas)

        return {
            "filename": filename,
            "format": ext,
            "chunks_added": len(chunk_ids),
            "total_chars": len(clean_text),
            "chunk_ids": chunk_ids,
        }

    @classmethod
    def process_file_from_path(cls, file_path: str) -> dict:
        """从文件路径处理"""
        path = Path(file_path)
        with open(file_path, "rb") as f:
            file_bytes = f.read()
        return cls.process_file(file_bytes, path.name)


# 全局文档处理器
document_processor = DocumentProcessor()
