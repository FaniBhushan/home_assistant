---
name: planning
description: Structure implementation roadmaps, milestone tracking, and task breakdowns. Use this when defining phases, writing TODO lists, or creating/updating execution plans.
---

# Planning Skill

This skill guides the creation of development roadmaps, milestones, and task checklists to ensure incremental, low-risk progress.

## Guidelines for Planning

1. **Phased Staging**:
   - Break projects down into distinct phases (e.g., Software-only, Local Hardware, Dedicated Production).
   - Ensure each phase has a clear set of verification checks before proceeding to the next.
2. **Task breakdowns (`task.md`)**:
   - Translate approved plans into actionable checklists.
   - Use standard formats:
     - `[ ]` for uncompleted tasks.
     - `[/]` for in-progress tasks.
     - `[x]` for completed tasks.
3. **Refinement of Plans**:
   - Keep implementation plans dynamic. If a design choice changes (e.g., swapping Android for iPad), update the relevant design documents (`hardware_blueprint.md` or `ROADMAP.md`) first to maintain documentation integrity.
4. **Minimizing Regression Risk**:
   - Always plan for verification steps (unit tests, manual curl commands, UI validation) at each milestone.
