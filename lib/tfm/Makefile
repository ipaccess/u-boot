# -DTFM_CHECK
ccflags-$(CONFIG_TFM) += -DTFM_ALREADY_SET -DTFM_NO_ASM -DTFM_TIMING_RESISTANT

TFM_OBJS = \
    fp_read_unsigned_bin.o \
    fp_to_unsigned_bin.o \
    fp_unsigned_bin_size.o \
    fp_count_bits.o \
    fp_reverse.o \
    fp_div_2d.o \
    fp_mod_2d.o \
    fp_rshd.o \
    fp_exptmod.o \
    fp_montgomery_setup.o \
    fp_montgomery_calc_normalization.o \
    fp_montgomery_reduce.o \
    fp_cmp_mag.o \
    fp_mod.o \
    fp_mulmod.o \
    fp_mul.o \
    fp_sqr.o \
    fp_2expt.o \
    fp_set.o \
    fp_mul_2.o \
    s_fp_sub.o \
    fp_div.o \
    fp_add.o \
    fp_mul_comba.o \
    fp_sqr_comba_generic.o \
    fp_mul_2d.o \
    fp_lshd.o \
    fp_sub.o \
    fp_cmp.o \
    fp_mul_d.o \
    s_fp_add.o

obj-$(CONFIG_TFM) += $(TFM_OBJS)
