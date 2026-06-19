#!/usr/bin/env python3
"""
Validates the L3 "App Configuration: Kconfig" assignment.

Checks the *structure* of the student's Kconfig file against the required
menu tree (LED Subsystem) rather than hardcoding symbol names, so it works
regardless of what the student named things -- it matches on type, nesting,
ranges and prompt keywords instead.

Required menu structure (from the assignment):

    [*] LED Subsystem  --->
         LED blink sleep time (1s (medium))  --->
         [ ] Advanced LED settings  --->
              (100) LED brightness (0-100)
              (500) LED fade duration (ms)
              Expert settings  --->
                   [ ] Enable LED debugging
                   [ ] Custom blink pattern

Usage:
    KCONFIG_PATH=path/to/Kconfig python3 check_kconfig_l3.py

Exits 0 if all checks pass, 1 otherwise. Prints a PASS/FAIL checklist either way.
"""
import os
import sys

import kconfiglib as kc

RESULTS = []  # list of (bool, str)


def check(condition, label):
    RESULTS.append((bool(condition), label))
    return bool(condition)


def iter_tree(node):
    """Depth-first walk: descends into node.list, then continues via node.next."""
    while node:
        yield node
        if node.list:
            yield from iter_tree(node.list)
        node = node.next


def children(node):
    """Direct children of `node` only (does not descend into grandchildren)."""
    out = []
    n = node.list
    while n:
        out.append(n)
        n = n.next
    return out


def descendants(node):
    """All nodes nested under `node`, not including node itself or its siblings."""
    return list(iter_tree(node.list)) if node.list else []


def subtree(node):
    return [node] + descendants(node)


def prompt_text(node):
    return node.prompt[0] if node.prompt else None


def has_text(node, *words):
    p = prompt_text(node)
    if not p:
        return False
    p = p.lower()
    return all(w.lower() in p for w in words)


def is_real_menuconfig(node):
    """True only for an actual 'menuconfig SYM' (not 'menu', not 'choice')."""
    return isinstance(node.item, kc.Symbol) and node.is_menuconfig


def is_choice(node):
    return isinstance(node.item, kc.Choice)


def is_plain_menu(node):
    return node.item is kc.MENU


def find(nodes, pred):
    return next((n for n in nodes if pred(n)), None)


def find_all(nodes, pred):
    return [n for n in nodes if pred(n)]


def main():
    path = os.environ.get("KCONFIG_PATH", "app/Kconfig")

    if not os.path.isfile(path):
        print(f"::error::Kconfig file not found at '{path}'. "
              f"Set KCONFIG_PATH if your Kconfig lives elsewhere.")
        return 1

    os.environ.setdefault("srctree", os.path.dirname(os.path.abspath(path)) or ".")
    try:
        kconf = kc.Kconfig(path, warn=False)
    except Exception as exc:  # noqa: BLE001 - want to report any parse error, not crash
        print(f"::error::Kconfig failed to parse '{path}': {exc}")
        return 1

    # ---- 1. Top-level "LED Subsystem" menuconfig ---------------------------
    led = find(iter_tree(kconf.top_node),
               lambda n: is_real_menuconfig(n) and has_text(n, "led") and has_text(n, "subsystem"))

    if check(led is not None,
             "Top-level 'menuconfig' (not plain 'config') with a prompt mentioning 'LED Subsystem'"):
        check(led.item.type == kc.BOOL, "LED Subsystem symbol is type bool")
        led_scope = subtree(led)
    else:
        led_scope = list(iter_tree(kconf.top_node))  # fall back to whole tree so later checks can still run

    # ---- 2. choice for blink sleep time, 4 options -------------------------
    choice = find(led_scope, lambda n: is_choice(n) and (has_text(n, "sleep") or has_text(n, "blink")))

    if check(choice is not None, "'choice' block for LED blink sleep time, nested under LED Subsystem"):
        members = [n for n in children(choice) if isinstance(n.item, kc.Symbol)]
        check(len(members) == 4, f"choice has exactly 4 options (found {len(members)})")

        labels = " | ".join(prompt_text(m) or "" for m in members).lower()
        for token in ["250", "500", "1s", "2s"]:
            check(token in labels, f"a choice option mentions '{token}'")
    else:
        choice = None

    # ---- 3. hidden int symbol backing the choice ---------------------------
    hidden_int = find(
        led_scope,
        lambda n: isinstance(n.item, kc.Symbol) and n.item.type == kc.INT and n.prompt is None
    )
    check(hidden_int is not None,
          "a hidden int symbol (type int, no prompt) exists to hold the numeric sleep time")

    # ---- 4. "Advanced LED settings" menuconfig, nested ---------------------
    advanced = find(led_scope, lambda n: is_real_menuconfig(n) and has_text(n, "advanced"))

    if check(advanced is not None,
             "'menuconfig' for Advanced LED settings, nested under LED Subsystem"):
        check(advanced.item.type == kc.BOOL, "Advanced LED settings symbol is type bool")
        adv_scope = subtree(advanced)
    else:
        adv_scope = led_scope  # fall back so later checks can still report specific issues

    # ---- 5. brightness + fade duration ranges ------------------------------
    brightness = find(adv_scope, lambda n: isinstance(n.item, kc.Symbol)
                       and n.item.type == kc.INT and has_text(n, "bright"))
    if check(brightness is not None, "an int symbol for LED brightness under Advanced LED settings"):
        if check(len(brightness.item.ranges) > 0, "LED brightness has a 'range' declared"):
            lo, hi, _cond = brightness.item.ranges[0]
            check(int(lo.str_value) == 0 and int(hi.str_value) == 100,
                  f"LED brightness range is 0-100 (found {lo.str_value}-{hi.str_value})")

    fade = find(adv_scope, lambda n: isinstance(n.item, kc.Symbol)
                and n.item.type == kc.INT and has_text(n, "fade"))
    if check(fade is not None, "an int symbol for LED fade duration under Advanced LED settings"):
        if check(len(fade.item.ranges) > 0, "LED fade duration has a 'range' declared"):
            lo, hi, _cond = fade.item.ranges[0]
            check(int(lo.str_value) == 0 and int(hi.str_value) == 5000,
                  f"LED fade duration range is 0-5000 (found {lo.str_value}-{hi.str_value})")

    # ---- 6. "Expert settings" plain menu with 'visible if' -----------------
    expert = find(adv_scope, lambda n: is_plain_menu(n) and has_text(n, "expert"))

    if check(expert is not None,
             "a plain 'menu' (not 'menuconfig') for Expert settings, nested under Advanced LED settings"):
        check(expert.visibility is not kconf.y,
              "Expert settings menu has a 'visible if' condition (isn't unconditionally visible)")
        expert_scope = subtree(expert)
    else:
        expert_scope = adv_scope  # fall back so later checks can still report specific issues

    # ---- 7. Expert settings contents ---------------------------------------
    debug_sym = find(expert_scope, lambda n: isinstance(n.item, kc.Symbol)
                      and n.item.type == kc.BOOL and has_text(n, "debug"))
    check(debug_sym is not None, "a bool symbol for 'Enable LED debugging' under Expert settings")

    pattern_sym = find(expert_scope, lambda n: isinstance(n.item, kc.Symbol)
                        and n.item.type == kc.BOOL and has_text(n, "custom") and has_text(n, "pattern"))
    check(pattern_sym is not None, "a bool symbol for 'Custom blink pattern' under Expert settings")

    # ---- report -------------------------------------------------------------
    print("L3 Kconfig structure check")
    print("=" * 60)
    failed = 0
    for passed, label in RESULTS:
        print(f"[{'PASS' if passed else 'FAIL'}] {label}")
        if not passed:
            failed += 1
    print("=" * 60)
    print(f"{len(RESULTS) - failed}/{len(RESULTS)} checks passed")

    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())