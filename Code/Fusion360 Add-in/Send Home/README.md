# Send Home Fusion 360 Add-In

Send Home adds a Fusion 360 command that sends the active viewport to the default Home View.

## Install

Copy the entire `Send Home` directory to your Fusion 360 Add-Ins folder.

Typical macOS location:

```text
~/Library/Application Support/Autodesk/Autodesk Fusion 360/API/AddIns/Send Home
```

Typical Windows location:

```text
%APPDATA%\Autodesk\Autodesk Fusion 360\API\AddIns\Send Home
```

The folder should contain `Send Home.py`, `Send Home.manifest`, and the `resources` directory.

## Enable

1. Open Fusion 360.
2. Go to **Utilities > Add-Ins > Scripts and Add-Ins**.
3. Select the **Add-Ins** tab.
4. Select **Send Home**.
5. Click **Run**.
6. Keep **Run on Startup** enabled so the command is available after Fusion 360 restarts.

## Assign shortcut

Assign the `Send To Home View` command to `Cmd+Shift+N` on macOS.

This shortcut matches firmware that sends `Cmd+Shift+N` from a short press on GPIO27.

## Behavior

When executed, the command calls Fusion 360's active viewport Home View action and refreshes the viewport:

```python
activeViewport.goHome(True)
activeViewport.refresh()
```
