# Wuwa Tappo PS5 手柄兼容补丁

让使用 XInput 的 Windows 版 Wuwa Tappo 识别 PS5 DualSense 手柄，并响应按键、记录计数。

已在 Windows 11 x64 + 普通 DualSense 蓝牙连接上实测，使用者确认桌宠能识别并计数。

## 安装

1. 下载 Releases 中的补丁压缩包并解压。
2. 从系统托盘彻底退出 Wuwa Tappo。
3. 把 `xinput1_4.dll` 放在 `WuwaTappo.exe` 所在目录。
4. 重新打开桌宠，连接 PS5 手柄，按 ×、○、□、△ 或方向键验证动作与计数。

如果该目录已有同名 DLL，请先备份，避免覆盖其他补丁。请勿将文件放进 Windows 系统目录。

## 卸载

彻底退出桌宠，删除新增的 `xinput1_4.dll`，重新启动即可。原 EXE 无需修改。

## 工作原理

补丁在桌宠进程内读取 DualSense HID 输入，并通过 `XInputGetState` 提供兼容状态。不需要虚拟手柄驱动，也不向其他程序发送键鼠输入。原有 Xbox 输入优先转发到系统 XInput。

| PS5 输入 | XInput 输入 |
| --- | --- |
| × / ○ / □ / △ | A / B / X / Y |
| L1 / R1 | LB / RB |
| L2 / R2 | LT / RT |
| Create / Options | Back / Start |
| L3 / R3、方向键、左右摇杆 | 对应的标准 XInput 输入 |

## 支持范围与限制

- 64 位 Windows，目标程序导入 `xinput1_4.dll!XInputGetState`。
- 普通 DualSense（054C:0CE6）蓝牙连接已实测；USB 报告和 DualSense Edge（054C:0DF2）已有实现，尚未实机验证。
- 只提供一只 PS5 手柄，使用 XInput 0 号槽；该槽已有 Xbox 手柄时，Xbox 优先。
- PS 键、触摸板按压和静音键不映射；不提供震动、自适应扳机和灯光控制。
- 仅导出 `XInputGetState`，这是针对桌宠的兼容补丁，请勿作为通用游戏 XInput DLL 使用。
- 桌宠升级后可能需要重新验证。若系统加载策略跳过应用目录 DLL，本补丁不会生效。

## 从源码构建

使用 Zig 0.13.0，在项目目录运行：

```powershell
zig cc -target x86_64-windows-gnu -O2 -shared shim.c -o xinput1_4.dll -lhid -luser32
```

仓库中的 DLL 与本次实测版本一致。`SHA256SUMS.txt` 提供校验值。

这是非官方兼容补丁，与 Kuro Games、Sony 和 Microsoft 无关联。仓库不包含 Wuwa Tappo 原程序或素材。
