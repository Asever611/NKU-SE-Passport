"""
核心配置管理模块
从 .env 文件和环境变量加载系统配置
"""

import os
from pathlib import Path
from dotenv import load_dotenv

# 加载 .env 文件
BASE_DIR = Path(__file__).resolve().parent.parent.parent
load_dotenv(BASE_DIR / ".env")


class Config:
    """系统全局配置"""

    # LLM 配置
    LLM_API_KEY: str = os.getenv("LLM_API_KEY", "")
    LLM_API_BASE: str = os.getenv("LLM_API_BASE", "https://api.deepseek.com/v1")
    LLM_MODEL: str = os.getenv("LLM_MODEL", "deepseek-chat")

    # 嵌入模型
    EMBEDDING_MODEL: str = os.getenv("EMBEDDING_MODEL", "all-MiniLM-L6-v2")
    HF_ENDPOINT: str = os.getenv("HF_ENDPOINT", "https://hf-mirror.com")

    # 服务端口
    API_PORT: int = int(os.getenv("API_PORT", "8000"))
    UI_PORT: int = int(os.getenv("UI_PORT", "8501"))

    # 存储路径
    CHROMA_PERSIST_DIR: str = os.getenv("CHROMA_PERSIST_DIR", str(BASE_DIR / "data" / "chroma"))
    SQLITE_DB_PATH: str = os.getenv("SQLITE_DB_PATH", str(BASE_DIR / "data" / "app.db"))
    UPLOAD_DIR: str = os.getenv("UPLOAD_DIR", str(BASE_DIR / "data" / "uploads"))

    # 安全护栏
    BLOCKED_KEYWORDS: list = os.getenv("BLOCKED_KEYWORDS", "作弊,代考,替考,抄袭,泄题").split(",")

    # 文档处理参数
    CHUNK_SIZE: int = 1000          # 文本分块大小（字符数）
    CHUNK_OVERLAP: int = 200        # 文本分块重叠大小
    RETRIEVAL_TOP_K: int = 5        # 检索返回的Top-K文档块数

    @classmethod
    def validate(cls):
        """验证必要配置"""
        if not cls.LLM_API_KEY or cls.LLM_API_KEY == "your_api_key_here":
            raise ValueError(
                "请先设置 LLM_API_KEY！\n"
                "编辑项目根目录下的 .env 文件，将 your_api_key_here 替换为你的 API Key。"
            )

    @classmethod
    def ensure_dirs(cls):
        """确保数据目录存在"""
        Path(cls.CHROMA_PERSIST_DIR).mkdir(parents=True, exist_ok=True)
        Path(cls.UPLOAD_DIR).mkdir(parents=True, exist_ok=True)
        Path(cls.SQLITE_DB_PATH).parent.mkdir(parents=True, exist_ok=True)


config = Config()
