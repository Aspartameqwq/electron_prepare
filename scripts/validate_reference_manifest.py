#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""校验 docs/reference/REFERENCE_MANIFEST.yml 与参考治理一致性。

检查项（P0 门禁，失败即退出码 1）：
  1. YAML 可解析（PyYAML 必须安装，缺失=失败，不降级 SKIP）；
  2. references 为非空列表；
  3. 每个条目必填字段齐全（缺=失败）；
  4. archive_sha256 为 64 位十六进制；
  5. id / archive_path / review.path 无重复；
  6. archive_path 存在且 SHA-256（流式）匹配；
  7. 有 review 的条目：reviewed_archive_sha256 == archive_sha256；
  8. 绝对路径扫描（Windows 盘符 / C:\\Users / /home/<user>），剥离 URL、豁免规则文档 AGENTS.md/CLAUDE.md。

用法： python scripts/validate_reference_manifest.py
仓库根目录由 __file__ 推导，不依赖当前工作目录。
"""
import hashlib
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
MANIFEST = ROOT / "docs/reference/REFERENCE_MANIFEST.yml"

REQUIRED_ENTRY_FIELDS = (
    "id", "name", "category", "archive_path", "archive_sha256",
    "inventory_status", "review_status",
)
REQUIRED_LICENSE_FIELDS = (
    "content_owner", "usage_terms_status", "attribution_required",
    "evidence_path", "nested_third_party", "nested_license_status",
    "redistribution_status",
)
SHA_RE = re.compile(r"^[0-9a-fA-F]{64}$")

# 绝对路径模式（剥离 URL 后匹配）
# 扫描范围仅"治理文档"（.md/.yml）：机器路径作为内容提交的主要载体。
# 豁免规则文档（AGENTS/CLAUDE/PLAN 含"禁止提交"示例文本）与第三方参考库/临时日志。
ABS_PATH_RE = [
    re.compile(r"(?<![A-Za-z0-9_])[A-Za-z]:\\\\"),    # Windows 盘符 + 反斜杠，如 F:\TI
    re.compile(r"C:[\\\\/]Users[\\\\/]", re.I),
    re.compile(r"/home/[A-Za-z0-9_.\-]+"),
]
URL_RE = re.compile(r"https?://\S+")
EXEMPT_RULE_DOCS = {"AGENTS.md", "CLAUDE.md", "PLAN.md"}   # 规则文档含"禁止提交路径"示例
SCAN_GLOBS = ("*.md", "*.yml", "*.yaml")
SKIP_DIR_PREFIXES = ("examples_and_documents", ".git", "logs")  # 第三方库由 manifest 哈希治理


def sha256_stream(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def check_manifest() -> int:
    if not MANIFEST.exists():
        print(f"FAIL: manifest 不存在: {MANIFEST.relative_to(ROOT)}")
        return 1

    try:
        import yaml
    except ImportError:
        print("FAIL: PyYAML 未安装（请 pip install -r requirements-dev.txt）。不得降级 SKIP。")
        return 1

    try:
        data = yaml.safe_load(MANIFEST.read_text(encoding="utf-8"))
    except yaml.YAMLError as e:
        print(f"FAIL: YAML 解析错误\n{e}")
        return 1

    fails = 0
    if not isinstance(data, dict):
        print("FAIL: manifest 顶层不是映射")
        return 1
    for key in ("schema_version", "inventory_complete", "total_reference_count"):
        if key not in data:
            print(f"FAIL: 缺顶层字段 {key}")
            fails += 1
    refs = data.get("references")
    if not isinstance(refs, list) or not refs:
        print("FAIL: references 必须为非空列表")
        fails += 1
        refs = refs or []

    seen_ids, seen_paths, seen_reviews = set(), set(), set()
    for ref in refs:
        rid = ref.get("id", "<no-id>")
        # 必填字段
        for k in REQUIRED_ENTRY_FIELDS:
            if k not in ref:
                print(f"FAIL [{rid}]: 缺必填字段 {k}")
                fails += 1
        lic = ref.get("license")
        if not isinstance(lic, dict):
            print(f"FAIL [{rid}]: license 必须为映射")
            fails += 1
        else:
            for k in REQUIRED_LICENSE_FIELDS:
                if k not in lic:
                    print(f"FAIL [{rid}]: license 缺字段 {k}")
                    fails += 1
        # 哈希格式
        sha = ref.get("archive_sha256", "")
        if not SHA_RE.match(sha):
            print(f"FAIL [{rid}]: archive_sha256 不是 64 位十六进制")
            fails += 1
        # 去重
        if rid in seen_ids:
            print(f"FAIL [{rid}]: id 重复")
            fails += 1
        seen_ids.add(rid)
        ap = ref.get("archive_path")
        if ap:
            norm = ap.replace("\\", "/")
            if norm in seen_paths:
                print(f"FAIL [{rid}]: archive_path 重复 {ap}")
                fails += 1
            seen_paths.add(norm)
            p = ROOT / ap
            if not p.is_file():
                print(f"FAIL [{rid}]: 文件不存在 {ap}")
                fails += 1
            elif SHA_RE.match(sha):
                got = sha256_stream(p)
                if got.lower() != sha.lower():
                    print(f"FAIL [{rid}]: SHA-256 不匹配 {ap}\n  want {sha}\n  got  {got}")
                    fails += 1
                else:
                    print(f"OK   [{rid}]: {ap} (sha256 匹配)")
        # review 一致性
        rev = ref.get("review")
        if isinstance(rev, dict):
            rpath = rev.get("path", "")
            rnorm = rpath.replace("\\", "/")
            if rnorm in seen_reviews:
                print(f"FAIL [{rid}]: review.path 重复 {rpath}")
                fails += 1
            seen_reviews.add(rnorm)
            if rev.get("status") != "REVIEWED":
                print(f"FAIL [{rid}]: review.status 非 REVIEWED")
                fails += 1
            if rev.get("reviewed_archive_sha256") != ref.get("archive_sha256"):
                print(f"FAIL [{rid}]: reviewed_archive_sha256 != archive_sha256（评审可能对应旧 ZIP）")
                fails += 1
            if rpath and not (ROOT / rpath).is_file():
                print(f"FAIL [{rid}]: review 文件不存在 {rpath}")
                fails += 1
            else:
                fails += check_review_content(rid, rpath, sha)
    return 1 if fails else 0


VALID_CLASSIFICATIONS = (
    "SOURCE_FACT", "CALCULATED", "INFERRED", "HARDWARE_REQUIRED", "TARGET_PROJECT_DECISION",
)


def check_review_content(rid, rpath, want_sha) -> int:
    """已评审项的评审文档须含：source_id、ZIP SHA、评审日期、证据表 + 合法分类。"""
    if not rpath:
        return 0
    text = (ROOT / rpath).read_text(encoding="utf-8")
    fails = 0
    if rid not in text:
        print(f"FAIL [{rid}]: 评审文档缺 source_id {rid}")
        fails += 1
    if want_sha and want_sha.lower() not in text.lower():
        print(f"FAIL [{rid}]: 评审文档缺 ZIP SHA-256 {want_sha[:12]}...")
        fails += 1
    if "评审日期" not in text:
        print(f"FAIL [{rid}]: 评审文档缺评审日期")
        fails += 1
    if "## 证据表" not in text:
        print(f"FAIL [{rid}]: 评审文档缺 ## 证据表")
        fails += 1
    else:
        table_ok = any(c in text for c in VALID_CLASSIFICATIONS)
        if not table_ok:
            print(f"FAIL [{rid}]: 证据表缺合法分类值 {VALID_CLASSIFICATIONS}")
            fails += 1
    return fails


def check_absolute_paths() -> int:
    """扫描仓库文本文件中的绝对路径（剥离 URL，豁免规则文档）。"""
    fails = 0
    hits = []
    for p in ROOT.rglob("*"):
        if p.is_dir() or p.name in EXEMPT_RULE_DOCS:
            continue
        rel_parts = p.relative_to(ROOT).parts
        if any(rel_parts[0] == s for s in SKIP_DIR_PREFIXES):
            continue
        if any(p.name.endswith(g.lstrip("*")) for g in SCAN_GLOBS):
            rel = p.relative_to(ROOT).as_posix()
            try:
                text = p.read_text(encoding="utf-8", errors="replace")
            except OSError:
                continue
            for line_no, raw in enumerate(text.splitlines(), 1):
                line = URL_RE.sub("", raw)              # 剥离 URL
                for pat in ABS_PATH_RE:
                    if pat.search(line):
                        hits.append(f"{rel}:{line_no}: {raw.strip()[:120]}")
                        fails += 1
                        break
    for h in hits:
        print(f"ABS-PATH: {h}")
    if fails:
        print(f"FAIL: 发现 {fails} 处绝对路径")
    else:
        print("OK: 无绝对路径（已剥离 URL，豁免 AGENTS.md/CLAUDE.md 规则示例）")
    return 1 if fails else 0


def main() -> int:
    rc = 0
    rc |= check_manifest()
    rc |= check_absolute_paths()
    if rc:
        print("RESULT: FAILED")
        return 1
    print("RESULT: ALL OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
