# my-os
2021 Summer Semister OS 操作系统课程设计

## 环境配置
本项目使用bochs-2.4.5，书上提示2.3.5但是在操作的时候一直提示我缺少文件，无法完成make和sudo make install，最后实际使用2.4.5版本安装提示所缺插件后成功运行。
按照书上的配置信息把反汇编和debug模块装好然后运行 在linux计算机根目录下找到bochs目录 在bochsrc文件中修改两个模块的文件路径BIOS和VGABIOS为对应的路径 还有KEYMAP
之后在有a.img和bochsrc的具体文件夹中运行 bochs命令即可 或者bochs -f bochsrc
在显示正常运行后，会弹出黑框等待配置，此时只需要输入c即continue代表继续运行，bochs虚拟机即可以正常运行，环境配置结束。
