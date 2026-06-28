# Handoff: ue-blueprint-extractor Plugin — `modify_blueprint_graphs` Fix

## Mission

Fix `modify_blueprint_graphs` so it actually creates function body nodes (not just empty function stubs) when called with a DSL or a JSON payload with `nodes[]`/`connections[]`.

---

## Repository

**Location**: `E:\Projects\ue5\ue-blueprint-extractor`
**Status**: Open-source fork, no constraint on modifications.
**Test project for integration testing**: `E:\Projects\ue5\Aura` (UE5 GAS project)

---

## Architecture Overview

Two layers:

### TS Layer (MCP server)
- `MCP/src/tools/blueprint-authoring.ts` — Tool handler for `modify_blueprint_graphs`
  - Line 241: When `dsl` is provided, calls `parseBlueprintDsl()` + `blueprintDslToPayload()`, then sets `operation='upsert_function_graphs'`
  - Lines 263-268: Calls C++ via `callSubsystemJson('ModifyBlueprintGraphs', ...)`
- `MCP/src/helpers/blueprint-dsl-parser.ts` — DSL parser + converter
  - `parseBlueprintDsl()` (Line 73): Converts pseudocode DSL → `BlueprintDslGraph[]`
  - `blueprintDslToPayload()` (Line 401): Converts graphs → `{ functionGraphs: [{ graphName, nodes[], connections[] }] }`
  - `PayloadNode` (Line 50): Supports `K2Node_Event`, `K2Node_CallFunction`, `K2Node_DynamicCast`, `K2Node_IfThenElse`, `K2Node_VariableGet`, `K2Node_VariableSet`, `K2Node_MacroInstance`, `K2Node_Literal`
  - `PayloadConnection` (Line 56): `{ fromNode, fromPin, toNode, toPin }`

### C++ Layer (Private/Authoring/)
- `BlueprintExtractor/Source/BlueprintExtractor/Private/Authoring/BlueprintAuthoring.cpp`
  - **`UpsertFunctionStubs()`** (Line 1382): Called when `operation='upsert_function_graphs'`. Only reads `functionName` (or `graphName`/`name`) and `category`/flags from each function object. **Completely ignores `nodes` and `connections` arrays.**
  - **`InsertExecNodes()`** (Line 1743): The function that CAN spawn real nodes (K2Node_CallFunction, K2Node_GetSubsystem). Works by BREAKING an existing exec connection and inserting new nodes in between.
  - **`FindNodeByTitle()`** (Line 1703): Utility used by `InsertExecNodes`. Uses `Contains()` match, returns first match only.
  - **`AppendFunctionCallToSequence()`** (Line 1499): Alternative approach — adds a call node to an existing ExecutionSequence node's then pin.
  - Main dispatcher at Line 3200+: routes operations like `upsert_function_graphs`, `insert_exec_nodes`, `append_function_call_to_sequence`, `replace_function_stubs`, `compile`.

---

## Root Causes Found

### Root Cause 1: C++ ignores nodes/connections from DSL payload
- TS builds a rich payload: `{ functionGraphs: [{ graphName: "MCP_NewFunc_B", nodes: [...], connections: [...] }] }`
- C++ `UpsertFunctionStubs` (Line 1382-1449) only does:
  1. Line 1428-1442: `CreateNewGraph` + `AddFunctionGraph` → creates function shell (Entry + Return nodes)
  2. Line 1445: `ApplyFunctionStub` → sets category + flags on Entry node
- **The `nodes[]` and `connections[]` arrays are never read.**

### Root Cause 2: `insert_exec_nodes` requires existing exec connections
- `InsertExecNodes` (Line 1743) works by:
  1. Finding existing connected pins via `insertAfter`/`insertBefore`
  2. Breaking the connection
  3. Spawning new nodes in between
  4. Wiring exec chain: Source.then → newNode.execute → newNode.then → ... → Target.execute
- It cannot create nodes in an empty graph or add nodes without an existing chain to break.
- It only supports `K2Node_CallFunction` and `K2Node_GetSubsystem` (Line 2077-2081).

### Root Cause 3: `FindNodeByTitle` cannot distinguish Entry from Return
- Both `K2Node_FunctionEntry` and `K2Node_FunctionResult` return the function name as `GetNodeTitle(ENodeTitleType::ListView)`.
- E.g., both show `"MCP_NewFunc_B"`.
- `FindNodeByTitle("MCP_NewFunc_B")` always returns the Entry node (first match).
- Since Entry has `"then"` pin but no `"execute"` pin, `insertBefore: { nodeTitle: "MCP_NewFunc_B", pinName: "execute" }` fails — it finds Entry, but Entry has no `"execute"` pin.
- **`insert_exec_nodes` is fundamentally broken for function graphs** because the Return node is unreachable by title.

### Root Cause 4: `replace_function_stubs` crashes
- `ReplaceFunctionStubs` (Line 1304) removes ALL existing function graphs, then recreates them.
- The RemoveGraphs call causes crashes, likely due to stale graph references elsewhere in the Blueprint.

---

## Fix Options

### Option A (Recommended) — Fix `FindNodeByTitle` + wire DSL to `insert_exec_nodes`

**C++ changes (BlueprintAuthoring.cpp):**
1. Add optional `nodeClass` / `class` filter to `FindNodeByTitle()` — around Line 1703
2. Read optional `sourceNodeClass` / `targetNodeClass` from `insertAfter` / `insertBefore` in `InsertExecNodes()` — around Line 1783
3. Pass `nodeClass` to `FindNodeByTitle` when provided — this lets you match `K2Node_FunctionResult` for insertBefore

**TS changes (blueprint-dsl-parser.ts + blueprint-authoring.ts):**
1. After `upsert_function_graphs` creates the function shell, make a SECOND call to `insert_exec_nodes` to spawn the actual body nodes
2. The `insertAfter` should reference the Entry node (by function name, pin: "then")
3. The `insertBefore` should reference the Return node (by function name + nodeClass: "K2Node_FunctionResult", pin: "execute")
4. Convert DSL's `nodes[]` and `connections[]` into the `insert_exec_nodes` format

### Option B — Use `append_function_call_to_sequence` instead
- Requires the function graph to already have a Sequence node
- Adds a call node to the last Then pin
- Very limited — only appends, can't build complex graphs

### Option C — Extend `upsert_function_graphs` to actually process nodes
- Modify the C++ `UpsertFunctionStubs` (or create a new operation) to read `nodes[]`/`connections[]`
- Would need to spawn nodes and wire them, similar to `InsertExecNodes` but with broader node class support

---

## Key Code References

### C++ (BlueprintAuthoring.cpp)
| Function | Line | Role |
|---|---|---|
| `ApplyFunctionStub` | 1272 | Sets category + flags on Entry node |
| `ReplaceFunctionStubs` | 1304 | Removes ALL function graphs + recreates (crashes) |
| `UpsertFunctionStubs` | 1382 | Creates function graph + applies stub (ignores nodes/connections) |
| `AppendFunctionCallToSequence` | 1499 | Adds call to Sequence node's Then pin |
| `FindGraphIncludingCollapsed` | 1641 | Graph lookup across FunctionGraphs/UbergraphPages/collapsed |
| `FindNodeByTitle` | 1703 | Node lookup by title Contains() — first match only |
| `FindPinByName` | 1481 | Pin lookup by name |
| `InsertExecNodes` | 1743 | Spawns nodes between existing exec connections |
| Main dispatch (switch on operation) | 3200+ | Routes upsert_function_graphs / insert_exec_nodes / etc. |
| `ApplyCreatePayload` | 3267 | Create blueprint: variables + components + functions + settings |
| `ReplaceFunctionStubs` (create path) | 1304-1380 | Create-only version used by `ApplyCreatePayload` |

**Supported node classes for insert_exec_nodes** (Line 1952-2081):
- `K2Node_CallFunction` — with `functionName` (self member) or `functionReference` (external member)
- `K2Node_GetSubsystem` — with `subsystemClass`

**All other node classes in the DSL parser** (Line 554-598) are NOT supported by `insert_exec_nodes`:
- `K2Node_Event`, `K2Node_DynamicCast`, `K2Node_IfThenElse`, `K2Node_VariableGet`, `K2Node_VariableSet`, `K2Node_MacroInstance`, `K2Node_Literal`

### TS Files
| File | Line | Role |
|---|---|---|
| `MCP/src/tools/blueprint-authoring.ts` | 185-280 | Tool handler, DSL→upsert_function_graphs routing |
| `MCP/src/helpers/blueprint-dsl-parser.ts` | 1-649 | Full DSL parser + converter |
| `MCP/src/helpers/blueprint-dsl-parser.ts` | 401-424 | `blueprintDslToPayload` — builds `{ functionGraphs: [...] }` |
| `MCP/src/helpers/blueprint-dsl-parser.ts` | 445-500 | `convertNodeTree` — DSL node → payload node + connections |
| `MCP/src/helpers/blueprint-dsl-parser.ts` | 554-599 | `buildPayloadNode` — type dispatch for DSL→PayloadNode |
| `MCP/src/tools/blueprint-authoring.ts` | 241-251 | DSL detected → auto-sets operation to `upsert_function_graphs` |
| `MCP/src/tools/blueprint-authoring.ts` | 263-268 | Calls C++ via `callSubsystemJson('ModifyBlueprintGraphs', ...)` |

---

## Test Blueprint (from this session)

**Asset**: `/Game/UI/MCP_Test_Blueprint.MCP_Test_Blueprint`
**Created function**: `MCP_NewFunc_B`
**Result**: Function shell created (visible in Blueprint editor), but body is empty → crash at runtime.

**Test execution logs** confirming the issue:
- `ModifyBlueprintGraphs completed successfully` (no error)
- `UpsertFunctionStubs created function graph 'MCP_NewFunc_B' and applied stub`
- But extraction showed only `K2Node_FunctionEntry`, no body nodes

---

## Suggested Skills for Next Agent

- `handoff` — if continuing in new session
- `explore` — for wider codebase searches across the plugin

---

## Effort Estimate

| Component | Lines to change | Difficulty |
|---|---|---|
| C++: `FindNodeByTitle` + `InsertExecNodes` nodeClass filter | ~20 lines | Easy |
| C++: Add more nodeClass support to `InsertExecNodes` | ~100-150 lines | Medium |
| TS: Reroute DSL→insert_exec_nodes (two-pass) | ~50-100 lines | Medium |
| Total | ~1-2 person-days | Medium |

---

## Relevant Paths (absolute)

- `E:\Projects\ue5\ue-blueprint-extractor\BlueprintExtractor\Source\BlueprintExtractor\Private\Authoring\BlueprintAuthoring.cpp`
- `E:\Projects\ue5\ue-blueprint-extractor\MCP\src\tools\blueprint-authoring.ts`
- `E:\Projects\ue5\ue-blueprint-extractor\MCP\src\helpers\blueprint-dsl-parser.ts`
- `E:\Projects\ue5\ue-blueprint-extractor\MCP\src\server-config.ts`
- `E:\Projects\ue5\ue-blueprint-extractor\MCP\src\tool-surface-manager.ts`
