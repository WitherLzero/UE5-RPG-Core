# GameplayTag Reference Browser — 实现报告

> **状态：v2 实现完成** ✅
> C++ 后端 + 蓝图前端均已实现并验证。功能可用，文档与实际情况一致。

---

## 1. 方案概述

| 区域 | v2 方案 | 状态 |
|---|---|---|
| Tag 选择 | 点击 Trigger (Border) 弹出 `SGameplayTagPicker` 原生弹窗，多选树形 | ✅ **完成** |
| 已选 Tag 展示 | `SelectTags` (ScrollBox) + `WBP_TagChip`（自移除，带 `×` 按钮） | ✅ **完成** |
| 引用结果 | 底部 `AssetListView` 显示 CDO 扫描资产 + `OpenViewerButton` 触发独立 Reference Viewer 标签页 | ✅ **完成** |
| 精确引用 | `OpenReferenceViewerForGameplayTags` — 每个 tag 独立窗口 | ✅ **完成**（ForEach 每个 tag 单开） |
| Include Children | `CheckBox` + `bIncludeChildren` 变量，展开父 Tag 所有子 Tag | ✅ **完成** |
| Tag Chip 移除 | Chip 自调用 `Remove from Parent` + `Event Dispatcher` 通知 EUW 同步 `SelectedTags` | ✅ **完成** |
| 状态栏 | 显示 `"{n} tags selected."` / `"{n} tags total."` | ✅ **完成** |
| 水平滚动 | `AssetSummaryPanel` 内 `AssetListView` 外套水平 `ScrollBox`，完整路径可拖拽查看 | ✅ **完成** |

---

## 2. C++ 实现（`RPGCoreEditor` 模块）

### 2.1 `OpenGameplayTagPicker`

- **声明：** `Source/RPGCoreEditor/Public/GameplayTagBrowser/RPGCoreEditorFunctionLibrary.h`
- **实现：** `Source/RPGCoreEditor/Private/GameplayTagBrowser/RPGCoreEditorFunctionLibrary.cpp`
- 弹出模态 `SGameplayTagPicker`，支持多选、Filter
- 关闭后返回 `TArray<FString>`

### 2.2 `OpenReferenceViewerForGameplayTags`

- 路径同上
- 遍历 `TagStrings`，为每个 tag 创建 `FAssetIdentifier(FGameplayTag::StaticStruct(), TagName)`
- 若 `bIncludeChildren`，对每个 tag 调用 `RequestGameplayTagChildren`，将后代也加入 root 列表
- 每个 tag **单独调用 `FEditorDelegates::OnOpenReferenceViewer`**，打开独立标签页
- 如果 `TagStrings` 为空，不操作

### 2.3 保留的 v1 CDO 扫描函数（`RPGCore` 模块）

- `GetAllGameplayTagStrings`
- `GetGameplayTagCount`
- `FindAssetsReferencingGameplayTags`
- `FindAssetListItemsReferencingGameplayTags`

---

## 3. 蓝图资产

### 3.1 `WBP_TagChip` — 已集成并投入使用

**路径：** `/Game/EditorUtilities/GameplayTagBrowser/WBP_TagChip`

```
CanvasPanel (Root)
└── SizeBox (RootSizeBox) [160×34]
    └── HorizontalBox (ChipHBox)
        ├── TextBlock (TagNameText) [Font 11, Bold]
        └── Button (DeselectButton) [Text: "×"]
```

**变量：** `TagString` (String, Public)

**逻辑：**
- `DeselectButton.OnClicked` → `Remove from Parent` (UI 移除) + `OnRemoveClicked.Broadcast(TagString)`
- Event Dispatcher: `OnRemoveClicked` (参数：TagString)

### 3.2 `WBP_TagRow` — v1 遗留，不再使用

**路径：** `/Game/EditorUtilities/GameplayTagBrowser/WBP_TagRow`

```
HorizontalBox_29
├── CheckBox (TagCheckBox)
└── TextBlock (TagText)
```

### 3.3 `WBP_AssetListItem` — ListView 使用中

**路径：** `/Game/EditorUtilities/GameplayTagBrowser/WBP_AssetListItem`

```
SizeBox_22
└── TextBlock (TagText) [Font 12, Bold]
```

### 3.4 `EUW_GameplayTagBrowser` — 最终面板

**路径：** `/Game/EditorUtilities/GameplayTagBrowser/EUW_GameplayTagBrowser`
**父类：** `EditorUtilityWidget`
**根节点：** `CanvasPanel_37`

```
CanvasPanel_37 (Root) [Full Screen Anchors, 16px Offsets]
│
└── Container (VerticalBox)
    │
    ├── TopBar (HorizontalBox) [Auto Height]
    │   │
    │   ├── SelectionArea (HorizontalBox) [0.5 Fill]
    │   │   ├── TextBlock_162 — "Tags:" [14, Bold]
    │   │   └── SizeBox_197 [WidthOverride=300]
    │   │       └── Trigger (Border) [var, 近黑背景]
    │   │           └── SizeBox_975 [MaxDesiredHeight=96]
    │   │               └── SelectTags (EditorUtilityScrollBox) [var]
    │   │
    │   └── ButtonArea (HorizontalBox) [0.5 Fill, HAlign_Right, VAlign_Top]
    │       ├── IncludeChildrenOpt (EditorUtilityCheckBox) [var]
    │       ├── TextBlock_266 — "Include Children" [14, Bold]
    │       ├── OpenViewerButton (EditorUtilityButton) [var]
    │       │   └── TextBlock_330 — "Search References" [12, Bold]
    │       └── ClearAllButton (EditorUtilityButton) [var]
    │           └── TextBlock_2 — "Clear All" [12, Bold]
    │
    ├── BottomBar (HorizontalBox) [0.6 Fill]
    │   │
    │   ├── AssetSummaryPanel (VerticalBox) [0.4 Fill]
    │   │   ├── TextBlock — "Asset Summary (CDO scan)" [16, Bold]
    │   │   └── ScrollBox (Horizontal)
    │   │       └── AssetListView (EditorUtilityListView) [var, EntryWidget=WBP_AssetListItem]
    │   │
    │   └── ViewerPlaceholder (Border) [var, 0.6 Fill]
    │
    └── StatusText (TextBlock) [var, 14, Bold]
```

**变量清单：**

| 变量名 | 类型 | 用途 |
|---|---|---|
| `AllTags` | String Array | 所有 tag 列表（CDO 扫描） |
| `SelectedTags` | String Array | 当前已选 Tags |
| `TagCounts` | Integer | tag 总数 |
| `SelectedTagCounts` | Integer | 已选 tag 数 |
| `bIncludeChildren` | Bool | 是否包含子 Tag |
| `StatusText` | TextBlock | 状态栏 |
| `Trigger` | Border | 点击打开 Tag Picker 的触发区域 |
| `SelectTags` | EditorUtilityScrollBox | Chip 容器 |
| `IncludeChildrenOpt` | EditorUtilityCheckBox | 包含子 Tag |
| `OpenViewerButton` | EditorUtilityButton | 打开 Reference Viewer |
| `ClearAllButton` | EditorUtilityButton | 清空选择 |
| `AssetListView` | EditorUtilityListView | CDO 扫描结果列表 |
| `ViewerPlaceholder` | Border | 占位区域 |

---

## 4. 蓝图逻辑

### 4.1 PreConstruct 绑定

```
PreConstruct
  └→ Sequence
      ├→ Bind IncludeChildrenOpt.OnCheckStateChanged → Set bIncludeChildren
      ├→ Bind OpenViewerButton.OnClicked → OnClicked_Event
      ├→ Bind AssetListView.OnItemDoubleClicked → BP_OnItemDoubleClicked_Event
      └→ Bind ClearAllButton.OnClicked → OnClicked_Event_0
```

### 4.2 Construct

```
Construct
  → GetAllGameplayTagStrings → Set AllTags
  → Length(AllTags) → Set TagCounts
  → UpdateStatusText()
```

### 4.3 Tag Picker 打开

```
Trigger.OnMouseButtonDownEvent → On_Trigger_MouseButtonDown
  → OpenGameplayTagPicker(InitialTags=SelectedTags) → Set SelectedTags
  → RefreshTagChips()
```

### 4.4 RefreshTagChips

```
RefreshTagChips (Custom Event)
  → Clear Children(SelectTags)
  → ForEach (SelectedTags)
      → CreateSingleTagChip(TagName) → chip (Output pin)
      → Bind Event(chip.OnRemoveClicked → OnChipRemoved_Event)
```

### 4.5 CreateSingleTagChip（函数）

```
CreateSingleTagChip(InString) → Output: SelectedTagChip (WBP_TagChip)
  → Create Widget(WBP_TagChip)
  → Set Chip.TagString = InString
  → Set Chip.TagNameText.Text = InString
  → Add Child(SelectTags, Chip)
  → Return Chip as SelectedTagChip
```

### 4.6 OnChipRemoved_Event

```
OnChipRemoved_Event (TagName)
  → RemoveItem(SelectedTags, TagName)
  → UpdateStatusText()
```

### 4.7 OpenViewerButton.OnClicked

```
OnClicked_Event
  → ClearListItems(AssetListView)
  → FindAssetListItemsReferencingGameplayTags(SelectedTags) → ForEach → AddItem
  → ForEach(SelectedTags)
      → OpenReferenceViewerForGameplayTags([单个tag], bIncludeChildren)
```

### 4.8 ClearAllButton.OnClicked

```
OnClicked_Event_0
  → Clear(SelectedTags)
  → ClearListItems(AssetListView)
  → RefreshTagChips()
```

### 4.9 BP_OnItemDoubleClicked_Event

```
AssetListView.OnItemDoubleClicked
  → Cast To RPGEditorListItemData → Get DisplayString
  → Make Array [DisplayString]
  → SyncBrowserToObjects(AssetPaths)
```

### 4.10 UpdateStatusText（函数）

```
Branch(SelectedTagCounts > 0)
  → True:  Set StatusText.Text = "{SelectedTagCounts} tags selected."
  → False: Set StatusText.Text = "{TagCounts} tags total."
```

---

## 5. 验证清单

- [x] `RPGCoreEditor` 模块编译通过
- [x] Trigger 区域点击弹出原生 `SGameplayTagPicker`，可多选，支持 Filter
- [x] Picker 选择后关闭，`SelectTags` ScrollBox 显示 `WBP_TagChip` 列表（每行带 `×` 按钮）
- [x] Chip 的 `×` 按钮点击后，UI 移除该 Chip，`SelectedTags` 数组同步删除，状态栏更新
- [x] `IncludeChildren` 勾选后 `bIncludeChildren` 变量正确更新
- [x] `OpenViewerButton` 点击：清除旧列表 → CDO 扫描填充 `AssetListView` → 每个 tag 打开独立 Reference Viewer
- [x] `ClearAll` 清空 `SelectedTags`、`AssetListView`、刷新 Chip 列表
- [x] 底部 `AssetSummaryPanel` 水平滚动条可用，可完整查看资产路径
- [x] 双击 `AssetListView` 条目 → `SyncBrowserToObjects` 定位到 Content Browser
- [x] StatusText 显示当前选中 tag 数，选中为空时显示总数

---

## 6. 已知限制

### UE Reference Viewer 多 root 行为

内置 `SReferenceViewer` 的 `SetGraphRootIdentifiers` 将多个输入打包为单个 `"Multiple Items"` 节点。如果需要在一个面板中**每个 tag 独立一棵树**（TagA → [Refs], TagB → [Refs] 并列显示），必须自定义 Slate widget 替代。

当前方案折衷：每个 tag 调用一次 `FEditorDelegates::OnOpenReferenceViewer`，打开独立的 Reference Viewer 标签页。

### StatusText 可简化

当前 `TagCounts` / `SelectedTagCounts` 两个变量仅用于状态栏显示，可合并为动态 `Length(SelectedTags)` 计算。

---

## 7. 后续可能方向

- **自定义 Reference Viewer 面板**：替代 `ViewerPlaceholder`，嵌入 EUW，实现多 root 并列显示
- **Tag Filter 输入框**：传到 `.Filter` 限制 picker 范围
- **导出报告**：SelectedTags + 引用资产列表导出 CSV
- **RemoveItem 后同步**：当前 `OnChipRemoved` 已同步状态栏和数组，如需同步 `AssetListView` 可追加 `FindAssetListItemsReferencingGameplayTags`

---

## 8. 参考节点

| 节点 | 来源 |
|---|---|
| `Open Gameplay Tag Picker` | RPGCoreEditor |
| `Open Reference Viewer For Gameplay Tags` | RPGCoreEditor |
| `Find Asset List Items Referencing Gameplay Tags` | RPGCore |
| `Clear Children` | UMG |
| `Create Widget` | UMG |
| `Add Child` | UMG |
| `RemoveItem` (数组) | KismetArrayLibrary |
| `Clear List Items` | ListView |
| `Add Item` | ListView |
| `Sync Browser to Objects` | EditorAssetLibrary |

---

*最后更新：2026-06-28 — v2 实现完成，文档与实际一致*
