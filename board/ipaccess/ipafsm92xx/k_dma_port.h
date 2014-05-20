
#define BIT(nr) (1UL << (nr))

#define writel_relaxed(v,c) writel(v,c)
#define readl_relaxed(c) readl(c)
#define rmb(){}


typedef int irqreturn_t;
