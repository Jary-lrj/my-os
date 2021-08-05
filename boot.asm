;%define _BOOT_DEBUG_		; 制作 Boot Sector 时一定将此行注释掉!
				; 去掉此行注释后可做成.COM文件易于调试:
				;   nasm Boot.asm -o Boot.com 

%ifdef  _BOOT_DEBUG_
	org  0100h		
%else
	org  07c00h		
%endif

	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	call	DispStr		
	jmp	$		
DispStr:
	mov	ax, BootMessage
	mov	bp, ax		
	mov	cx, 16		
	mov	ax, 01301h	
	mov	bx, 000ch	
	mov	dl, 0
	int	10h		
	ret
BootMessage:		db	"Welcome to Use Jary-li operate system"
times 	510-($-$$)	db	0
dw 	0xaa55			  
