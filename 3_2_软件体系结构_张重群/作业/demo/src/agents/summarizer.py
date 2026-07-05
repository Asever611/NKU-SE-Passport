"""
SummarizerAgent (总结智能体)
职责: 结构化知识总结 —— 从课件提取关键知识点, 生成层次化笔记
应用设计模式: Prompt Chaining (提示链模式)
"""

from src.agents.base import BaseAgent
from src.core.knowledge_base import knowledge_base


class SummarizerAgent(BaseAgent):
    """知识总结智能体 — Prompt Chaining 三阶段"""

    def __init__(self):
        super().__init__("SummarizerAgent", "知识总结归纳")

    def _has_real_content(self, text: str) -> bool:
        """检测输出是否包含真实知识点（而非LLM关于'空内容'的幻觉）"""
        if not text or len(text) < 20:
            return False
        # 幻觉关键词：LLM 在无有效输入时常见的元讨论
        hallucination_markers = [
            "空知识列表", "如何处理空", "未提供任何", "数据校验层",
            "反馈与引导", "输入缺失", "阻塞流程", "快速失败",
            "校验规则", "短路机制", "validate_knowledge",
            "没有提供", "无法进行", "空输入", "边界情况",
        ]
        for marker in hallucination_markers:
            if marker in text:
                return False
        return True

    def process(self, input_data: dict) -> dict:
        query = input_data.get("query", "")
        history = input_data.get("history", [])

        kb_stats = knowledge_base.get_collection_stats()
        if kb_stats["total_chunks"] == 0:
            return {
                "response": "知识库中还没有课件内容，请先上传课件后再使用总结功能！",
                "sources": [],
            }

        all_chunks = knowledge_base.get_all_chunks(max_chars=8000)
        if not all_chunks.strip():
            return {
                "response": "知识库中暂无课件内容，请先上传课件。",
                "sources": [],
            }

        sources = [d["filename"] for d in kb_stats.get("documents", [])]
        content_len = len(all_chunks)
        num_points = min(max(content_len // 300, 3), 20)

        # === Stage 1: 关键点提取 ===
        stage1_output = self.llm.chat_with_retry(
            [{"role": "user", "content": (
                f"以下是用户上传的课件内容。请提取其中 {num_points} 个关键知识点。\n\n"
                "# 课件内容:\n"
                f"{all_chunks[:5000]}\n\n"
                "# 要求:\n"
                "只输出知识点列表，每个一行，格式: 编号. 知识点名称 - 简要说明\n"
                "直接输出知识点，不要说'以下是提取的知识点'之类的开场白。"
            )}],
            temperature=0.3, max_tokens=2000,
        )

        if stage1_output.startswith("[LLM Error]"):
            return {"response": f"总结生成失败: {stage1_output}", "sources": sources}

        if not self._has_real_content(stage1_output):
            return {
                "response": (
                    "总结生成失败：模型未能从课件中提取有效的知识点。\n\n"
                    "这可能是因为课件内容格式特殊，请尝试：\n"
                    "1. 用更具体的描述提问，如'总结课件中关于XX的内容'\n"
                    "2. 直接在对话框输入'请帮我总结XX'\n"
                    "3. 确认上传的课件包含可读的文字内容"
                ),
                "sources": sources,
            }

        # === Stage 2: 结构化归纳 ===
        stage2_output = self.llm.chat_with_retry(
            [{"role": "user", "content": (
                "将以下知识点按主题分组成2-4个大类，形成层次化大纲。\n\n"
                f"知识点列表:\n{stage1_output}\n\n"
                "输出格式:\n"
                "## 大类标题\n"
                "- 知识点A\n"
                "- 知识点B\n\n"
                "只输出大纲本身，不要加解释。"
            )}],
            temperature=0.4, max_tokens=3000,
        )

        if stage2_output.startswith("[LLM Error]"):
            return {"response": f"归纳阶段失败: {stage2_output}", "sources": sources}

        if not self._has_real_content(stage2_output):
            # stage2 验证失败，直接返回 stage1 的结果作为降级
            return {
                "response": (
                    "## 📚 课件知识要点\n\n"
                    f"{stage1_output}\n\n"
                    "---\n"
                    "💡 以上为从课件中提取的核心知识点。如需更详细的层次化笔记，请重新尝试或上传更完整的课件。"
                ),
                "sources": [{"source": s} for s in sources],
            }

        # === Stage 3: 精细化输出 ===
        stage3_output = self.llm.chat_with_retry(
            [{"role": "user", "content": (
                "将以下大纲扩展为完整的学习笔记，每个知识点包含定义和关键要点。\n\n"
                f"{stage2_output}\n\n"
                "要求: Markdown格式、中文、最后加'📌 重点回顾'。只展开大纲中的知识点。"
            )}],
            temperature=0.5, max_tokens=4096,
        )

        if stage3_output.startswith("[LLM Error]"):
            # 降级返回 stage2
            return {"response": stage2_output, "sources": [{"source": s} for s in sources]}

        if not self._has_real_content(stage3_output):
            return {"response": stage2_output, "sources": [{"source": s} for s in sources]}

        return {
            "response": stage3_output,
            "sources": [{"source": s} for s in sources],
        }
