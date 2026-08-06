#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成 docs/reference/REFERENCE_MANIFEST.yml 完整库存。

扫描 examples_and_documents/ 下所有已入库参考文件：
  - 模块移植代码 ZIP（68 个）
  - 板级资料（PDF/HTML/ZIP/PACK）
自动计算 SHA-256（流式）、分配稳定 id、生成完整 schema。
5 个已评审项保留精化字段（sdk/sysconfig/review/license 分层）。

用法： python scripts/generate_reference_manifest.py
依赖： PyYAML（见 requirements-dev.txt）
"""
import hashlib
import pathlib

import yaml

ROOT = pathlib.Path(__file__).resolve().parent.parent
DOCS = ROOT / "examples_and_documents"
MODULES_DIR = DOCS / "立创·天猛星MSPM0G3507开发板【模块移植代码】"
MATERIALS_DIR = DOCS / "立创·天猛星MSPM0G3507开发板资料"

WIKI_MODULE_INDEX = "https://wiki.lckfb.com/zh-hans/tmx-mspm0g3507/module/"
WIKI_BOARD_INDEX = "https://wiki.lckfb.com/zh-hans/tmx-mspm0g3507/"

# 已评审模块：文件夹名 -> 稳定 id
REVIEWED_FOLDERS = {
    "TB6612电机驱动模块": "lckfb-tmx-tb6612",
    "N20直流减速电机-带霍尔编码器": "lckfb-tmx-n20-encoder",
    "SG90舵机": "lckfb-tmx-sg90",
    "0.96寸IIC单色屏": "lckfb-tmx-oled-ssd1306-iic",
    "MPU6050六轴传感器": "lckfb-tmx-mpu6050",
}
CAT_KEYS = {"控制类": "control", "传感器类": "sensor", "无线通信类": "wireless", "显示类": "display"}


def sha256_stream(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def review_block(rid: str):
    """已评审项的评审元数据（手动维护，与评审文档一致）。"""
    dates = {
        "lckfb-tmx-tb6612": "2026-08-04",
        "lckfb-tmx-n20-encoder": "2026-08-04",
        "lckfb-tmx-sg90": "2026-08-04",
        "lckfb-tmx-oled-ssd1306-iic": "2026-08-04",
        "lckfb-tmx-mpu6050": "2026-08-04",
    }
    files = {
        "lckfb-tmx-tb6612": "docs/example_reviews/tb6612.md",
        "lckfb-tmx-n20-encoder": "docs/example_reviews/n20-hall-encoder.md",
        "lckfb-tmx-sg90": "docs/example_reviews/sg90.md",
        "lckfb-tmx-oled-ssd1306-iic": "docs/example_reviews/oled-ssd1306-iic.md",
        "lckfb-tmx-mpu6050": "docs/example_reviews/mpu6050.md",
    }
    return {
        "path": files[rid],
        "status": "REVIEWED",
        "reviewed_at": dates[rid],
        "reviewed_archive_sha256": "PLACEHOLDER",  # 生成后回填为实际 ZIP 哈希
    }


def main() -> int:
    refs = []
    module_count = 0
    material_count = 0
    seq = 0

    # ---- 模块移植代码 ZIP ----
    for cat in sorted(MODULES_DIR.iterdir()):
        if not cat.is_dir():
            continue
        cat_key = CAT_KEYS.get(cat.name, cat.name)
        for mod in sorted(cat.iterdir()):
            z = mod / "TMX_MSPM0G3507_ModuleCode.zip"
            if not z.is_file():
                continue
            module_count += 1
            seq += 1
            rid = REVIEWED_FOLDERS.get(mod.name) or f"lckfb-tmx-module-{seq:03d}"
            rel = z.relative_to(ROOT).as_posix()
            entry = {
                "id": rid,
                "name": mod.name,
                "category": f"module-{cat_key}",
                "source_page_url": WIKI_MODULE_INDEX,
                "source_page_status": "MODULE_INDEX_ONLY",  # 模块总页，非精确下载链接
                "download_url": None,
                "download_url_status": "UNKNOWN",
                "downloaded_at": "2026-08-04",
                "archive_path": rel,
                "archive_sha256": sha256_stream(z),
                "inventory_status": "INVENTORIED",
                "review_status": "REVIEWED" if rid in REVIEWED_FOLDERS.values() else "NOT_REVIEWED",
                "license": {
                    "content_owner": "LCKFB",
                    "usage_terms_status": "REVIEW_REQUIRED",
                    "attribution_required": True,
                    "evidence_path": None,
                    "nested_third_party": "InvenSense-DMP" if "MPU6050" in mod.name else None,
                    "nested_license_status": "LICENSE_MISSING" if "MPU6050" in mod.name else None,
                    "redistribution_status": "REVIEW_REQUIRED",
                },
            }
            if rid in REVIEWED_FOLDERS.values():
                entry["sdk_version"] = "mspm0_sdk@2.02.00.05"
                entry["sysconfig_version"] = "1.21.0+3721"
                entry["review"] = review_block(rid)
            refs.append(entry)

    # ---- 板级资料 ----
    for f in sorted(MATERIALS_DIR.rglob("*")):
        if not f.is_file():
            continue
        material_count += 1
        seq += 1
        rel = f.relative_to(ROOT).as_posix()
        refs.append({
            "id": f"lckfb-tmx-material-{seq:03d}",
            "name": f.name,
            "category": "board-material",
            "source_page_url": WIKI_BOARD_INDEX,
            "source_page_status": "BOARD_INDEX_ONLY",
            "download_url": None,
            "download_url_status": "UNKNOWN",
            "downloaded_at": "2026-08-04",
            "archive_path": rel,
            "archive_sha256": sha256_stream(f),
            "inventory_status": "INVENTORIED",
            "review_status": "NOT_REVIEWED",
            "license": {
                "content_owner": "LCKFB-TI" if f.suffix.lower() in (".pdf",) else "LCKFB",
                "usage_terms_status": "REVIEW_REQUIRED",
                "attribution_required": True,
                "evidence_path": None,
                "nested_third_party": None,
                "nested_license_status": None,
                "redistribution_status": "REDISTRIBUTION_REVIEW_REQUIRED",
            },
        })

    # 回填已评审项的 reviewed_archive_sha256 == archive_sha256
    for r in refs:
        if "review" in r:
            r["review"]["reviewed_archive_sha256"] = r["archive_sha256"]

    data = {
        "schema_version": 1,
        "inventory_complete": True,
        "reviewed_reference_count": len([r for r in refs if r.get("review_status") == "REVIEWED"]),
        "total_reference_count": len(refs),
        "inventory_updated": "2026-08-04",
        "references": refs,
    }

    out = ROOT / "docs/reference/REFERENCE_MANIFEST.yml"
    with out.open("w", encoding="utf-8") as f:
        yaml.safe_dump(data, f, allow_unicode=True, sort_keys=False, default_flow_style=False)
    print(f"生成 {out.relative_to(ROOT)}: 模块 {module_count}, 资料 {material_count}, 合计 {len(refs)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
