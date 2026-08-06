#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""P0 文档一致性检查。

检查项（供 CI / 人工运行）：
  1. 已跟踪文档无 "v7.1" 残留（历史记录除外）；
  2. AGENTS.md 引用 v7.2；
  3. 六种阶段状态在 PLAN / README / AGENTS 中存在；
  4. skill 不再被描述为"规范层"（应为可选辅助）。

用法： python scripts/check_doc_consistency.py
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
FILES = [ROOT / "AGENTS.md", ROOT / "README.md", ROOT / "CLAUDE.md", ROOT / "PLAN.md"]
STATES = ("NOT_STARTED", "IN_PROGRESS", "COMPLETED", "SOFTWARE_READY", "BLOCKED", "FAILED")
STATE_RE = re.compile(r"(NOT_STARTED|IN_PROGRESS|COMPLETED|SOFTWARE_READY|BLOCKED|FAILED)")


def main() -> int:
    fails = 0

    def check(cond, msg):
        nonlocal fails
        if not cond:
            print("FAIL:", msg)
            fails += 1
        else:
            print("OK:  ", msg)

    text_ag = (ROOT / "AGENTS.md").read_text(encoding="utf-8")
    text_plan = (ROOT / "PLAN.md").read_text(encoding="utf-8")
    text_readme = (ROOT / "README.md").read_text(encoding="utf-8")
    text_claude = (ROOT / "CLAUDE.md").read_text(encoding="utf-8")

    # 1. v7.1 残留
    bad = []
    for f in FILES:
        if "v7.1" in f.read_text(encoding="utf-8"):
            bad.append(f.name)
    check(not bad, f"无 v7.1 残留（当前文件）" + (f"：{bad}" if bad else ""))

    # 2. AGENTS 引用 v7.2
    check("v7.2" in text_ag, "AGENTS.md 引用 v7.2")

    # 3. 六状态在 PLAN/README/AGENTS
    for fname, text in (("PLAN.md", text_plan), ("README.md", text_readme), ("AGENTS.md", text_ag)):
        present = set(STATE_RE.findall(text))
        missing = [s for s in STATES if s not in present]
        check(not missing, f"{fname} 含全部六状态" + (f"，缺 {missing}" if missing else ""))

    # 4. skill 不为"规范层"
    bad_skill = ("skill 为开发规范层" in text_claude or "skill 规范开发" in text_readme
                 or "规范层.*mspm0-ccs" in text_readme)
    check(not bad_skill, "skill 不再被描述为规范层（应为可选辅助）")

    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
