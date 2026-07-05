"""
智能学习助手 — Streamlit 前端界面
"""

import time
import streamlit as st
import requests
import uuid

st.set_page_config(
    page_title="智能学习助手",
    page_icon="🎓",
    layout="wide",
    initial_sidebar_state="expanded",
)

API = "http://localhost:8000/api"


# ===== 工具函数 =====
@st.cache_data(ttl=2, show_spinner=False)
def _cached_kb(_ts: int):
    try:
        r = requests.get(f"{API}/knowledge", timeout=5)
        return r.json() if r.status_code == 200 else None
    except Exception:
        return None


def kb_stats():
    return _cached_kb(int(time.monotonic() / 2))


@st.cache_data(ttl=3, show_spinner=False)
def _cached_sessions(_ts: int):
    try:
        r = requests.get(f"{API}/sessions", timeout=5)
        return r.json()["sessions"] if r.status_code == 200 else []
    except Exception:
        return []


def session_list():
    return _cached_sessions(int(time.monotonic() / 3))


def api(endpoint: str, method: str = "GET", **kwargs):
    try:
        url = f"{API}{endpoint}"
        r = (requests.get if method == "GET" else
             requests.post if method == "POST" else
             requests.delete)(url, timeout=5 if method != "POST" else 60, **kwargs)
        return r.json() if r.status_code == 200 else None
    except Exception:
        return None


# ===== 会话初始化 =====
if "sid" not in st.session_state:
    st.session_state.sid = str(uuid.uuid4())
if "msgs" not in st.session_state:
    st.session_state.msgs = []
if "processed" not in st.session_state:
    st.session_state.processed = set()
if "upload_key" not in st.session_state:
    st.session_state.upload_key = 0


# ===== 回调 =====
def cb_new_session():
    st.session_state.sid = str(uuid.uuid4())
    st.session_state.msgs = []
    _cached_sessions.clear()
    st.session_state["_switch_to"] = st.session_state.sid


def cb_switch_session(sid: str):
    st.session_state["_switch_to"] = sid


def cb_delete_session(sid: str):
    api(f"/sessions/{sid}", method="DELETE")
    _cached_sessions.clear()
    if st.session_state.sid == sid:
        cb_new_session()


def cb_delete_all_sessions():
    sessions = session_list()
    for s in (sessions or []):
        api(f"/sessions/{s['session_id']}", method="DELETE")
    _cached_sessions.clear()
    cb_new_session()


def cb_refresh():
    _cached_kb.clear()
    _cached_sessions.clear()


def cb_delete_doc(filename: str):
    api(f"/knowledge/{filename}", method="DELETE")
    st.session_state.processed.discard(filename)
    _cached_kb.clear()


def cb_delete_all_docs():
    s = kb_stats()
    if s and s.get("documents"):
        for d in s["documents"]:
            api(f"/knowledge/{d['filename']}", method="DELETE")
            st.session_state.processed.discard(d["filename"])
    _cached_kb.clear()


def cb_clear_chat():
    st.session_state.msgs = []


def cb_quick(prompt: str):
    st.session_state["_prompt"] = prompt


# ===== 处理会话切换 =====
switch_to = st.session_state.pop("_switch_to", None)
if switch_to:
    st.session_state.sid = switch_to
    # 从后端加载该会话的历史
    r = api(f"/history/{switch_to}")
    if r and r.get("messages"):
        st.session_state.msgs = r["messages"]
    else:
        st.session_state.msgs = []

# ============================================================
# 侧边栏
# ============================================================
with st.sidebar:
    st.markdown("## 🎓 智能学习助手")

    # ==== 会话管理 ====
    st.markdown("### 💬 会话")

    sessions = session_list()
    col_s1, col_s2 = st.columns(2)
    with col_s1:
        st.button("🆕 新建", on_click=cb_new_session, use_container_width=True)
    with col_s2:
        if sessions:
            st.button("🗑️ 清空全部", on_click=cb_delete_all_sessions,
                      use_container_width=True, help="删除所有历史会话")

    if sessions:
        with st.container(height=min(len(sessions) * 52 + 10, 200)):
            for s in sessions:
                is_current = s["session_id"] == st.session_state.sid
                prefix = "🔵 " if is_current else "⚪ "
                c1, c2 = st.columns([5, 1])
                with c1:
                    label = f"{prefix}{s['title']}"
                    st.button(
                        label,
                        key=f"sess_{s['session_id']}",
                        on_click=cb_switch_session,
                        args=(s['session_id'],),
                        use_container_width=True,
                        help=f"{s['msg_count']} 条消息 · {s['started']}",
                    )
                with c2:
                    st.button("✕", key=f"dels_{s['session_id']}",
                              on_click=cb_delete_session, args=(s['session_id'],),
                              help="删除此会话")

    st.markdown("---")

    # ==== 知识库状态 ====
    st.markdown("### 📚 知识库状态")
    stats = kb_stats()

    # 第一行：刷新 + 清空
    col_a, col_b = st.columns(2)
    with col_a:
        st.button("🔄 刷新", on_click=cb_refresh, use_container_width=True)
    with col_b:
        st.button("🗑️ 清空全部", on_click=cb_delete_all_docs,
                  use_container_width=True, disabled=not (stats and stats.get("documents")))
    # 第二行：文档数 + 切块数
    if stats:
        st.markdown(f"📄 **{stats['total_documents']}** 文档 ｜ 🧩 **{stats['total_chunks']}** 知识块")
    else:
        st.caption("点击刷新加载")

    if stats and stats.get("documents"):
        docs = stats["documents"]
        st.markdown("**课件列表:**")
        h = min(len(docs) * 44 + 10, 220)
        with st.container(height=h):
            for doc in docs:
                c1, c2 = st.columns([5, 1])
                with c1:
                    st.markdown(f"📄 {doc['filename']} `({doc['chunk_count']}块)`")
                with c2:
                    st.button("✕", key=f"dd_{doc['filename']}", help="删除",
                              on_click=cb_delete_doc, args=(doc['filename'],))

    st.markdown("---")

    # ==== 上传 ====
    st.markdown("### 📤 上传课件")

    uploaded = st.file_uploader(
        "支持 PDF/MD/TXT/DOCX，可多选",
        type=["pdf", "md", "txt", "docx"],
        accept_multiple_files=True,
        key=f"up_{st.session_state.upload_key}",
        label_visibility="collapsed",
    )

    if uploaded:
        new_files = [f for f in uploaded if f.name not in st.session_state.processed]
        if new_files:
            progress = st.progress(0, text="准备上传...")
            ok, fail = 0, 0
            for i, f in enumerate(new_files):
                progress.progress((i) / len(new_files), text=f"正在切块: {f.name}")
                r = api("/upload", method="POST", files={"file": (f.name, f.getvalue())})
                if r and r.get("success"):
                    st.session_state.processed.add(f.name)
                    ok += 1
                else:
                    fail += 1
            progress.progress(1.0, text=f"完成: {ok} 成功, {fail} 失败")
            progress.empty()  # 进度条自动消失
            if ok > 0:
                st.success(f"✅ 上传成功 {ok} 个文件，已完成切块与向量化")
                time.sleep(2)
            if fail > 0:
                st.error(f"❌ {fail} 个文件上传失败")
            _cached_kb.clear()
            st.session_state.upload_key += 1
            st.rerun()

    st.markdown("---")

    # ==== 快捷功能 ====
    st.markdown("### 🧭 快捷功能")
    c1, c2 = st.columns(2)
    c1.button("📖 总结", on_click=cb_quick, args=("请帮我总结知识库中课件的核心知识点",), use_container_width=True)
    c2.button("🎯 出题", on_click=cb_quick, args=("请根据知识库中的课件内容生成5道选择题",), use_container_width=True)
    c3, c4 = st.columns(2)
    c3.button("📋 计划", on_click=cb_quick, args=("请帮我制定一个7天复习计划，每天可以学习2小时",), use_container_width=True)
    c4.button("🗑️ 清空", on_click=cb_clear_chat, use_container_width=True, help="清空当前会话的聊天消息")

    st.markdown("---")
    st.caption(f"当前会话: {st.session_state.sid[:8]}...")

# ============================================================
# 主区域
# ============================================================
st.markdown(f"<h2 style='text-align:center;'>🎓 智能学习助手</h2>", unsafe_allow_html=True)
st.markdown("<p style='text-align:center;color:#888;'>上传课件 → 智能问答 · 知识总结 · 学习规划 · 自测练习</p>",
            unsafe_allow_html=True)

# 当前会话标题
cur_sessions = [s for s in (sessions or []) if s["session_id"] == st.session_state.sid]
if cur_sessions:
    st.caption(f"📌 当前会话: **{cur_sessions[0]['title']}** ({cur_sessions[0]['msg_count']} 条消息)")

# 对话历史
for msg in st.session_state.msgs:
    av = "🤖" if msg["role"] == "assistant" else "👤"
    with st.chat_message(msg["role"], avatar=av):
        st.markdown(msg.get("content", ""))

# 快捷功能
auto = st.session_state.pop("_prompt", None)
prompt = st.chat_input("输入你的问题...") or auto

if prompt:
    st.session_state.msgs.append({"role": "user", "content": prompt})
    with st.chat_message("user", avatar="👤"):
        st.markdown(prompt)

    with st.chat_message("assistant", avatar="🤖"):
        ph = st.empty()
        ph.caption("⏳ 思考中...")
        try:
            r = api("/chat", method="POST",
                    json={"query": prompt, "session_id": st.session_state.sid})
            if r:
                txt = r.get("response", "")
                src = r.get("sources", [])
                intent = r.get("intent", "")
                agent = r.get("agent", "")

                labels = {"question": "📚 智能问答", "summarize": "📝 知识总结",
                          "plan": "📋 学习规划", "quiz": "🎯 自测练习",
                          "greeting": "👋", "blocked": "🚫"}
                cap = labels.get(intent, intent)
                if agent:
                    cap += f" | {agent}"
                ph.caption(cap)
                st.markdown(txt)
                if src:
                    with st.expander("📚 引用来源"):
                        for s in src:
                            st.caption(f"- {s.get('source', '?')}")
                st.session_state.msgs.append({"role": "assistant", "content": txt, "sources": src})
            else:
                ph.error("后端未响应 — 请确认 FastAPI 已启动")
        except Exception as e:
            ph.error(f"错误: {e}")

    # 发完消息后刷新会话列表缓存
    _cached_sessions.clear()
