python
import os
def get_images_path():
  return os.getenv("NEMU_IMAGES_PATH")

def set_debug_paths(prefix, proj):
  gdb.execute(f'set substitute-path /build/{proj} -kernels {os.path.join(prefix, proj)}')

gdb.execute(f'file {get_images_path()}/../../libexec/am-kernels/am-tests')
end
set substitute-path /build/am-kernels /home/xin/repo/ysyx-workbench/am-kernels
set substitute-path /build/abstract-machine /home/xin/repo/ysyx-workbench/abstract-machine
target remote /tmp/gdbstub-nemu.sock
