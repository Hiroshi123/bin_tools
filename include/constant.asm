

;;; eflags carry flag
%define eflags_cf 0x0001
;;; eflags parity flag
%define eflags_pf 0x0004
;;; eflags adjust flag
%define eflags_af 0x0010
;;; eflags zero flag
%define eflags_zf 0x0040
;;; eflags sign flag
%define eflags_sf 0x0080
;;; eflags trap flag
%define eflags_tf 0x0100
;;; eflags interrupt enable flag
%define eflags_if 0x0200
;;; eflags direction flag
%define eflags_df 0x0400
;;; eflags overflow flag
%define eflags_of 0x0800
;;; eflags iopl
%define eflags_iopl 0x3000
;;; eflags nested task flag
%define eflags_nt 0x4000

;;; sign flag & overflow flag
%define eflags_sof 0x0880

;;; carry flag & zero flag
%define eflags_czf 0x0041
