"""
FastAPI 路由定义
暴露 REST API 供 Streamlit 前端调用
"""

import os
import shutil
from fastapi import APIRouter, UploadFile, File, HTTPException
from src.api.schemas import (
    ChatRequest, ChatResponse,
    UploadResponse, KnowledgeBaseStats,
    HealthResponse,
)
from src.agents.supervisor import supervisor
from src.core.knowledge_base import knowledge_base
from src.core.document_processor import document_processor
from src.core.config import config
from src.core.memory import memory_manager

router = APIRouter(prefix="/api")


@router.get("/health", response_model=HealthResponse)
async def health_check():
    """健康检查"""
    stats = knowledge_base.get_collection_stats()
    return HealthResponse(
        status="ok",
        version="1.0.0",
        kb_chunks=stats["total_chunks"],
        kb_documents=stats["total_documents"],
    )


@router.post("/chat", response_model=ChatResponse)
async def chat(request: ChatRequest):
    """
    智能对话接口
    接收用户问题，通过Supervisor Agent路由到对应专业Agent处理
    """
    result = await supervisor.process_with_memory(
        query=request.query,
        session_id=request.session_id,
    )
    return ChatResponse(
        query=request.query,
        intent=result.get("intent", "unknown"),
        agent=result.get("agent"),
        response=result.get("response", ""),
        sources=result.get("sources", []),
        plan=result.get("plan"),
        quiz=result.get("quiz"),
    )


@router.post("/upload", response_model=UploadResponse)
async def upload_document(file: UploadFile = File(...)):
    """
    文档上传接口
    接收课件文件，执行文档处理流水线，存入知识库
    """
    # 验证文件
    if not file.filename:
        raise HTTPException(status_code=400, detail="文件名不能为空")

    # 读取文件内容
    file_bytes = await file.read()
    if not file_bytes:
        raise HTTPException(status_code=400, detail="文件内容为空")

    # 保存原始文件
    config.ensure_dirs()
    file_path = os.path.join(config.UPLOAD_DIR, file.filename)
    with open(file_path, "wb") as f:
        f.write(file_bytes)

    # 处理文档
    try:
        result = document_processor.process_file(file_bytes, file.filename)
    except ValueError as e:
        return UploadResponse(
            success=False,
            filename=file.filename,
            error=str(e),
            message=f"格式不支持: {e}",
        )
    except Exception as e:
        return UploadResponse(
            success=False,
            filename=file.filename,
            error=str(e),
            message=f"处理失败: {e}",
        )

    if result.get("error"):
        return UploadResponse(
            success=False,
            filename=file.filename,
            error=result.get("error"),
            message=result.get("error"),
        )

    return UploadResponse(
        success=True,
        filename=result["filename"],
        format=result.get("format", ""),
        chunks_added=result["chunks_added"],
        total_chars=result.get("total_chars", 0),
        message=f"上传成功! 文档已解析为 {result['chunks_added']} 个知识块。",
    )


@router.get("/knowledge", response_model=KnowledgeBaseStats)
async def get_knowledge_stats():
    """获取知识库统计信息"""
    stats = knowledge_base.get_collection_stats()
    docs = [
        {"filename": d["filename"], "chunk_count": d["chunk_count"]}
        for d in stats.get("documents", [])
    ]
    return KnowledgeBaseStats(
        total_chunks=stats["total_chunks"],
        total_documents=stats["total_documents"],
        documents=docs,
    )


@router.delete("/knowledge/{filename}")
async def delete_document(filename: str):
    """删除指定文档"""
    deleted = knowledge_base.delete_document(filename)

    # 同时删除原始文件
    file_path = os.path.join(config.UPLOAD_DIR, filename)
    if os.path.exists(file_path):
        os.remove(file_path)

    return {"success": True, "deleted_chunks": deleted, "filename": filename}


@router.get("/sessions")
async def list_sessions():
    """列出所有历史会话"""
    sessions = await memory_manager.list_sessions()
    return {"sessions": sessions}


@router.delete("/sessions/{session_id}")
async def delete_session(session_id: str):
    """删除整个会话"""
    await memory_manager.delete_session(session_id)
    return {"success": True, "session_id": session_id}


@router.get("/history/{session_id}")
async def get_history(session_id: str):
    """获取会话历史"""
    history = await memory_manager.get_history(session_id)
    return {"session_id": session_id, "messages": history}
