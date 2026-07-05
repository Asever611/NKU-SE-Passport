"""
FastAPI 应用入口
负责启动后端服务, 注册路由和中间件

运行方式:
    cd 项目根目录
    uvicorn src.api.main:app --host 0.0.0.0 --port 8000 --reload

    或从 src/api 目录:
    python main.py
"""

import sys
from pathlib import Path

# 确保项目根目录在 Python 路径中 (解决从子目录运行时的模块导入问题)
PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from contextlib import asynccontextmanager
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from src.api.routes import router
from src.core.config import config
from src.core.memory import memory_manager


@asynccontextmanager
async def lifespan(app: FastAPI):
    """应用生命周期管理"""
    # 启动
    config.ensure_dirs()
    await memory_manager.init_db()
    print(f"[OK] Smart Learning Assistant API started")
    print(f"     API docs: http://localhost:{config.API_PORT}/docs")
    print(f"     KB path: {config.CHROMA_PERSIST_DIR}")
    yield
    # 关闭
    print("[OK] Smart Learning Assistant API shutdown")


# 创建 FastAPI 应用
app = FastAPI(
    title="Smart Learning Assistant API",
    description="Smart Learning Assistant & Knowledge Management Multi-Agent System",
    version="1.0.0",
    docs_url="/docs",
    redoc_url="/redoc",
    lifespan=lifespan,
)

# CORS 中间件
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# 注册路由
app.include_router(router)


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        app,  # 直接传 app 对象，避免模块路径查找问题
        host="0.0.0.0",
        port=config.API_PORT,
        reload=False,
    )
