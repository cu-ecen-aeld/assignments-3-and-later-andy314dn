# Faulty Oops Analysis

## Oops Log

```bash
$ echo "hello_world" > /dev/faulty 
Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
Mem abort info:
  ESR = 0x96000045
  EC = 0x25: DABT (current EL), IL = 32 bits
  SET = 0, FnV = 0
  EA = 0, S1PTW = 0
Data abort info:
  ISV = 0, ISS = 0x00000045
  CM = 0, WnR = 1
user pgtable: 4k pages, 39-bit VAs, pgdp=000000004211a000
[0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000
Internal error: Oops: 96000045 [#1] SMP
Modules linked in: hello(O) faulty(O) scull(O)
CPU: 0 PID: 151 Comm: sh Tainted: G           O      5.10.162-cip24 #1
Hardware name: linux,dummy-virt (DT)
pstate: 80000005 (Nzcv daif -PAN -UAO -TCO BTYPE=--)
pc : faulty_write+0x10/0x20 [faulty]
lr : vfs_write+0xf0/0x290
sp : ffffffc010d93dc0
x29: ffffffc010d93dc0 x28: ffffff80020fa700 
x27: 0000000000000000 x26: 0000000000000000 
x25: 0000000000000000 x24: 0000000000000000 
x23: 0000000000000000 x22: ffffffc010d93e00 
x21: 000000558848e990 x20: ffffff8002127900 
x19: 000000000000000c x18: 0000000000000000 
x17: 0000000000000000 x16: 0000000000000000 
x15: 0000000000000000 x14: 0000000000000000 
x13: 0000000000000000 x12: 0000000000000000 
x11: 0000000000000000 x10: 0000000000000000 
x9 : 0000000000000000 x8 : 0000000000000000 
x7 : 0000000000000000 x6 : 0000000000000000 
x5 : ffffff80022de5b0 x4 : ffffffc008737000 
x3 : ffffffc010d93e00 x2 : 000000000000000c 
x1 : 0000000000000000 x0 : 0000000000000000 
Call trace:
 faulty_write+0x10/0x20 [faulty]
 ksys_write+0x74/0x110
 __arm64_sys_write+0x1c/0x30
 el0_svc_common.constprop.0+0x94/0x1e0
 do_el0_svc+0x6c/0x90
 el0_svc+0x10/0x20
 el0_sync_handler+0xe8/0x120
 el0_sync+0x180/0x1c0
Code: d2800001 d2800000 d503233f d50323bf (b900003f) 
---[ end trace 9e5fe6f1a9fd4be1 ]---
```

## Analysis

### 🧠 Step-by-step Analysis

#### 1️⃣ Context

```bash
# echo "hello_world" > /dev/faulty
```

This means the user-space shell is writing to the device file `/dev/faulty`, which corresponds to a kernel module named `faulty`.
So the kernel will call the `write()` operation defined in that module’s `file_operations` struct.

#### 2️⃣ The crash

```bash
Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
```

That means the kernel tried to **dereference a NULL pointer (address 0x0)** — a write operation to an invalid memory address.

#### 3️⃣ Instruction state summary

```bash
ESR = 0x96000045
EC = 0x25: DABT (current EL), IL = 32 bits
WnR = 1   ← write access
```

The exception syndrome register (ESR) confirms a **Data Abort**, i.e., a **bad memory access**, while attempting to write (`WnR=1`).

#### 4️⃣ Important part

```bash
pc : faulty_write+0x10/0x20 [faulty]
```

At the time of the crash, the **program counter** (PC) was inside `faulty_write()` — specifically at offset `+0x10` from the start of that function.

So the crash happened **very early inside** `faulty_write()` (almost immediately).

#### 5️⃣ Disassembled code snippet

```bash
Code: d2800001 d2800000 d503233f d50323bf (b900003f)
```

That last instruction (`b900003f`) corresponds to a **store (STR)** instruction writing to address `0x0`
→ exactly what causes a `NULL` pointer dereference.

So yes — the crash is caused by an intentional write to `NULL` in the module’s code.

#### 6️⃣ Call trace

```bash
Call trace:
 faulty_write+0x10/0x20 [faulty]
 ksys_write+0x74/0x110
 __arm64_sys_write+0x1c/0x30
 el0_svc_common.constprop.0+0x94/0x1e0
 do_el0_svc+0x6c/0x90
 el0_svc+0x10/0x20
 el0_sync_handler+0xe8/0x120
 el0_sync+0x180/0x1c0
```

This is the **normal system call stack**:

```bash
user-space write()
→ sys_write()
→ vfs_write()
→ faulty_write()  ← crash here
```

#### 7️⃣ Module info

```bash
Modules linked in: hello(O) faulty(O) scull(O)
```

The above log confirms `faulty` is a **loadable kernel module (LKM)**.
The `O` flag (Out-of-tree) means it’s not part of the main kernel tree.

#### 8️⃣ Final conclusion

From the above analysis, we can see that the module purposely dereferences `NULL` to demonstrate kernel crash handling and oops tracing. It also helps us understand kernel `oops` messages to a certain level.
