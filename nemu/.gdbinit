python
import os
def get_images_path():
  return os.getenv("NEMU_IMAGES_PATH")

def set_debug_paths(prefix, proj):
  gdb.execute(f'set substitute-path /build/{proj} -kernels {os.path.join(prefix, proj)}')

def ps():
  gdb.execute('p/x *(uintptr_t *)$sp@20')

gdb.execute(f'file {get_images_path()}/../../libexec/am-kernels/am-tests')
end
# file /home/xin/repo/rt-thread-am/bsp/abstract-machine/build/rtthread.elf
file /nix/store/pg4ik2fsdb5ki708bkjvn5jdf225709x-am-kernel-riscv32-none-elf-2024-07-10/libexec/am-kernels/am-tests
set substitute-path /build/am-kernels /home/xin/repo/ysyx-workbench/am-kernels
set substitute-path /build/abstract-machine /home/xin/repo/ysyx-workbench/abstract-machine
# set debug remote 1
target remote /tmp/gdbstub-nemu.sock
