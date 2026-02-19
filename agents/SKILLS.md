# AEMU Agentic Skills Registry

This document provides a high-level overview of the specialized agents available in this workspace.
Each agent is defined by a `SKILL.md` file located in the `.skills/` directory.

| Agent | Role | Primary Responsibility |
| :--- | :--- | :--- |
| **[Scribe]** | Chronicler | Logs human interventions and identifies root causes. |
| **[Evolver]** | Architect | Analyzes logs and hardens skills to increase autonomy. |
| **[Planner]** | Strategist | Drafts `DESIGN.md` and performs atomic decomposition. |
| **[Critic]** | Auditor | Scrutinizes code for safety, performance, and alignment. |
| **[Debugger]** | Specialist | Uses GDB and logging to identify root causes of crashes. |
| **[Style]** | Enforcer | Ensures adherence to project-specific style guides. |
| **[Doc]** | Writer | Keeps `ARCHITECTURE.md` and code docs in sync. |
| **[License]** | Compliance | Validates and inserts the correct license preambles. |
| **[Commit]** | Release | Crafts high-quality, semantic commit messages. |
| **[Git]** | SCM | Handles rebases, conflict resolution, and repo hygiene. |
| **[Reviewer]** | Gatekeeper | Audits diffs for noise, metadata, and atomic principles. |

[Scribe]: .skills/agent_intervention_scribe/SKILL.md
[Evolver]: .skills/agent_autonomy_evolver/SKILL.md
[Planner]: .skills/planner/SKILL.md
[Critic]: .skills/critic/SKILL.md
[Debugger]: .skills/debugger/SKILL.md
[Style]: .skills/style_enforcer/SKILL.md
[Doc]: .skills/documenter/SKILL.md
[License]: .skills/license_agent/SKILL.md
[Commit]: .skills/committer/SKILL.md
[Git]: .skills/git_expert/SKILL.md
[Reviewer]: .skills/reviewer/SKILL.md

## Usage

To focus the lead agent's attention on a specific specialist's standards, you can explicitly
request their activation:

> "Activate the **Critic** and review this change for potential race conditions."

Alternatively, the lead agent will autonomously activate these skills as they progress through the
[Development Lifecycle](AGENTS.md#the-development-lifecycle).
