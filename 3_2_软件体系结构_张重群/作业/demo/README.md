# 🎓 智能学习助手与知识管理多智能体系统

> **软件体系结构课程大作业** — 智能体(Agent)系统体系结构设计与 Demo 实现

## 📖 项目简介

面向高校学生的**智能学习助手系统**，通过**多智能体协作**架构，将课件（PDF/Markdown/TXT/DOCX）整合为知识库，提供：

- 📚 **智能问答** — 基于课件内容的 RAG 问答，答案标注引用来源
- 📝 **知识总结** — 自动生成结构化笔记大纲（提示链模式）
- 📋 **学习规划** — 根据考试时间自动生成分阶段复习计划
- 🎯 **自测练习** — 根据知识点自动生成选择题/简答题

## 🏗️ 技术架构

| 层次 | 技术 | 说明 |
|------|------|------|
| 前端 | Streamlit | Web 交互界面 |
| 后端 API | FastAPI + Uvicorn | RESTful 服务 |
| Agent 框架 | LangChain + 自研框架 | 多智能体协作 |
| 向量数据库 | ChromaDB | 知识库语义检索 |
| 嵌入模型 | Sentence Transformers | 文本向量化 |
| 关系存储 | SQLite (aiosqlite) | 对话历史/学习记录 |
| LLM | DeepSeek API (兼容 OpenAI) | 大语言模型调用 |

**体系结构风格**：分层 + 数据为中心 + 管道-过滤器

**智能体设计模式**：Routing · RAG · Prompt Chaining · Multi-Agent · Planning

## 📁 项目结构

```
├── .env                    # API Key 配置文件
├── requirements.txt        # Python 依赖
├── README.md               # 项目说明
├── docs/
│   ├── 设计文档.md          # 完整设计文档（作业核心产出）
│   └── diagrams.html       # 架构图集（9张图，浏览器打开）
├── src/
│   ├── agents/             # Agent 模块
│   │   ├── base.py         # BaseAgent 抽象基类
│   │   ├── supervisor.py   # 监督者Agent（路由分发）
│   │   ├── retrieval.py    # 检索Agent（RAG问答）
│   │   ├── planner.py      # 规划Agent（学习计划）
│   │   ├── summarizer.py   # 总结Agent（提示链）
│   │   └── quiz.py         # 测验Agent（题目生成）
│   ├── core/               # 核心基础设施
│   │   ├── config.py       # 配置管理
│   │   ├── llm_service.py  # LLM调用封装
│   │   ├── knowledge_base.py  # ChromaDB知识库
│   │   ├── document_processor.py  # 文档处理流水线
│   │   └── memory.py       # 短期/长期记忆管理
│   ├── api/                # API 服务层
│   │   ├── main.py         # FastAPI 入口
│   │   ├── routes.py       # API 路由
│   │   └── schemas.py      # Pydantic 数据模型
│   └── ui/                 # 前端界面
│       └── app.py          # Streamlit 应用
└── data/                   # 数据存储（自动创建）
    ├── chroma/             # ChromaDB 向量数据
    └── uploads/            # 上传的原始文件
```

## 🚀 快速开始

### 1. 环境准备

推荐使用项目指定的 Python 环境：

```bash
# 激活环境 (Windows)
D:\Anaconda\envs\full\python.exe -m pip install --upgrade pip
```

### 2. 安装依赖

```bash
cd D:\Claude\Projects\SoftwareArchitecture3
pip install -r requirements.txt
```

### 3. 配置 API Key

编辑项目根目录下的 `.env` 文件，填入你的 API Key：

```env
LLM_API_KEY=sk-your-deepseek-api-key
LLM_API_BASE=https://api.deepseek.com/v1
LLM_MODEL=deepseek-chat
```

> 💡 也支持 OpenAI 兼容的其他模型，只需修改 `LLM_API_BASE` 和 `LLM_MODEL`。

### 4. 启动后端服务 (终端1)

```bash
cd src/api
python main.py
```

或：

```bash
uvicorn src.api.main:app --host 0.0.0.0 --port 8000 --reload
```

后端启动后访问 http://localhost:8000/docs 查看 API 文档。

### 5. 启动前端界面 (终端2)

```bash
cd src/ui
streamlit run app.py
```

前端启动后访问 http://localhost:8501 使用系统。

### 6. 使用流程

1. 在侧边栏**上传课件**（支持 PDF/MD/TXT/DOCX）
2. 在聊天框**输入问题**，AI 会基于课件回答
3. 使用侧边栏**快速功能**：总结知识、生成题目、制定计划

## 🧪 Demo 演示要点

演示视频建议展示以下核心功能（1-3分钟）：

1. **文档上传** → 上传一份课件，展示知识库更新
2. **智能问答** → 基于刚上传的课件提问，展示 RAG 能力和引用来源
3. **知识总结** → 使用"总结课件核心知识点"，展示提示链输出
4. **学习规划** → 输入"帮我制定7天复习计划"，展示规划Agent
5. **自测练习** → 生成5道选择题并展示答案解析

## 📊 设计文档

完整的设计文档见 `docs/设计文档.md`，包含：

- 用户需求分析（功能性 + 非功能性需求）
- 功能分解（6大模块，模块间依赖关系）
- 软件体系风格设计（分层 + 数据为中心 + 管道-过滤器）
- 智能体设计模式分析（Routing · RAG · Prompt Chaining · Multi-Agent）
- 4+1 视图分析（逻辑/开发/进程/物理/场景视图）

架构图集见 `docs/diagrams.html`，用浏览器打开即可查看全部 9 张图。

## ⚙️ 核心设计决策

| 决策 | 选择 | 理由 |
|------|------|------|
| 主体系结构风格 | 分层风格 | 清晰的关注点分离，适合Web-Agent架构 |
| 数据共享方式 | 数据为中心(ChromaDB) | Agent松耦合，通过共享知识库间接协作 |
| 文档处理 | 管道-过滤器 | ETL场景天然匹配，每阶段独立可替换 |
| Agent通信 | Supervisor路由 | 集中路由+分布式执行，灵活可扩展 |
| 前端框架 | Streamlit | 快速原型开发，Python原生，适合Demo |
| LLM集成 | OpenAI兼容API | 通用接口，支持DeepSeek/OpenAI等多模型 |

## ⚠️ 注意事项

- 首次运行时需要下载 Sentence Transformers 模型（约 80MB），请确保网络通畅
- 确保 `.env` 文件中已配置有效的 API Key
- ChromaDB 数据和上传的课件存储在 `data/` 目录下，请勿手动删除

## 📄 许可

本项目为课程大作业，仅供学习参考。
