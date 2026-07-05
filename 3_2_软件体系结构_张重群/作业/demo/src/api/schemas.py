"""
Pydantic 数据模型
定义API请求/响应的数据结构
"""

from pydantic import BaseModel, Field
from typing import List, Optional
from datetime import datetime


# ============ 请求模型 ============

class ChatRequest(BaseModel):
    """聊天/提问请求"""
    query: str = Field(..., description="用户输入的问题或指令", min_length=1, max_length=5000)
    session_id: str = Field(default="default", description="会话ID，用于记忆管理")
    history: List[dict] = Field(default_factory=list, description="对话历史")


class UploadRequest(BaseModel):
    """文档上传请求 (通过multipart/form-data上传文件)"""
    filename: Optional[str] = Field(None, description="文件名(可选)")


class DeleteDocRequest(BaseModel):
    """删除文档请求"""
    filename: str = Field(..., description="要删除的文件名")


# ============ 响应模型 ============

class Source(BaseModel):
    """引用来源"""
    source: str = Field(..., description="来源文件名")
    relevance: Optional[float] = Field(None, description="相关度分数")

class ChatResponse(BaseModel):
    """聊天/提问响应"""
    query: str
    intent: str = Field(..., description="识别的意图")
    agent: Optional[str] = Field(None, description="处理的Agent名称")
    response: str = Field(..., description="回复内容(Markdown格式)")
    sources: List[Source] = Field(default_factory=list, description="引用来源列表")
    plan: Optional[dict] = Field(None, description="学习计划(如果intent=plan)")
    quiz: Optional[dict] = Field(None, description="题目数据(如果intent=quiz)")
    timestamp: datetime = Field(default_factory=datetime.now)


class DocumentInfo(BaseModel):
    """文档信息"""
    filename: str
    chunk_count: int

class KnowledgeBaseStats(BaseModel):
    """知识库统计"""
    total_chunks: int
    total_documents: int
    documents: List[DocumentInfo] = Field(default_factory=list)


class UploadResponse(BaseModel):
    """文档上传响应"""
    success: bool
    filename: str
    format: Optional[str] = None
    chunks_added: int = 0
    total_chars: int = 0
    message: str = ""
    error: Optional[str] = None


class HealthResponse(BaseModel):
    """健康检查响应"""
    status: str
    version: str = "1.0.0"
    kb_chunks: int = 0
    kb_documents: int = 0
