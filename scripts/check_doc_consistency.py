#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""P0/P1 文档一致性检查。

检查项（供 CI / 人工运行）：
  1. 已跟踪文档无 "v7.1" 残留（历史记录除外）；
  2. AGENTS.md 引用 v7.2；
  3. 六种阶段状态在 PLAN / README / AGENTS 中存在；
  4. skill 不再被描述为"规范层"（应为可选辅助）；
  5. README/PLAN 对 P0/P1/P1A/P2/P3 的状态声明与 STATUS 一致；
  6. 阶段推进后不得残留过时断言（P0 进行中 / 尚未可编译 / E3=80MHz 等）；
  7. ERRATA 无 "待查" 残留、SysConfig warning 白名单措辞存在。

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
    text_status = (ROOT / "docs" / "STATUS.md").read_text(encoding="utf-8")
    text_errata = (ROOT / "docs" / "ERRATA_CHECKLIST.md").read_text(encoding="utf-8")

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

    # 5. 阶段状态跨文档一致性（STATUS 为准，README/PLAN 不得声明冲突状态）
    status_map = {}
    for line in text_status.splitlines():
        m = re.match(r"\| (P\d+[A-Z]?) \| \*\*(NOT_STARTED|IN_PROGRESS|COMPLETED|SOFTWARE_READY|BLOCKED|FAILED)\*\*", line)
        if m:
            status_map[m.group(1)] = m.group(2)
    for phase, st in sorted(status_map.items()):
        for fname, text in (("README.md", text_readme), ("PLAN.md", text_plan)):
            # 同一阶段在其他文档被声明为不同状态时失败（匹配 "P1:*状态*" / "P1=*状态*" 形式）
            conflicting = [c for c in re.findall(
                rf"{phase}\s*[:：=]\s*\*{{0,2}}(NOT_STARTED|IN_PROGRESS|COMPLETED|SOFTWARE_READY|BLOCKED|FAILED)",
                text) if c != st]
            if conflicting:
                check(False, f"{fname} 中 {phase} 声明 {conflicting} 与 STATUS({st}) 冲突")
            else:
                check(True, f"{phase}={st} 在 {fname} 无冲突声明")

    # 6. 过时断言检测（阶段推进后不得残留"尚无/未存在/旧状态"类矛盾句）
    stale_patterns = [
        (r"P0 进行中", "README 声称 P0 进行中（已 COMPLETED）"),
        (r"尚未开始任何 CCS", "声称未开始 CCS（P1 已建可编译工程）"),
        (r"尚未开始任何板端", "声称未开始板端（P1 已上板验证）"),
        (r"尚未形成可编译工程", "声称 firmware 尚未形成可编译工程（已可编译）"),
        (r"无 CCS 最小工程", "声称无 CCS 工程（P1 已建）"),
        (r"无 `?control\.syscfg`?", "声称无 control.syscfg（已存在）"),
        (r"E3 80MHz|80MHz 优化|E3 ＝? ?80MHz", "E3 被描述为 80MHz 优化（应为性能/功耗/编译优化）"),
        (r"System Reset ×1", "仍写 System Reset ×1（真实已 ×3）"),
    ]
    for fname, text in (("STATUS.md", text_status), ("README.md", text_readme)):
        for pat, desc in stale_patterns:
            if re.search(pat, text):
                check(False, f"{fname}: 过时断言 — {desc}")
            else:
                check(True, f"{fname}: 无过时断言（{pat}）")

    # 6b. README P1/P1A 现状总述不得再书写"尚未..."句式（§四 当前段）
    if re.search(r"尚未开始任何|尚未形成可编译", text_readme):
        check(False, "README §四/当前段仍含'尚未开始/尚未形成可编译'")
    else:
        check(True, "README 当前段无'尚未开始/尚未形成可编译'")

    # 7. ERRATA 无 "待查"（非法状态）残留，且 SysConfig warning 白名单门禁措辞存在
    #    只禁止"表内状态格 = 待查"（如 `| ... | 待查（P2） |`），不禁止"待查不是合法状态"的说明句。
    if re.search(r"\|\s*\*{0,2}待查", text_errata):
        check(False, "ERRATA_CHECKLIST 表内仍以 '待查' 作状态（P1 全部按使用核查完毕）")
    else:
        check(True, "ERRATA_CHECKLIST 无 '待查' 状态残留")
    if "新增任意 warning 即 BUILD FAILED" in text_errata:
        check(True, "ERRATA 保留 SysConfig warning 精确白名单门禁措辞")
    else:
        check(False, "ERRATA 缺少 SysConfig warning 精确白名单门禁措辞")

    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())