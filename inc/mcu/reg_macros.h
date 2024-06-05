#ifndef REG_MACROS_H
#define REG_MACROS_H

#define GET_REG(REG, PIN) (REG ## PIN)
#define GET_REG_BIT0(REG, PIN) (REG ## PIN ## _0)
#define GET_REG_BIT1(REG, PIN) (REG ## PIN ## _1)
#define GET_AFRL_REG(REG, PIN) (REG ## PIN ## _Pos)
// Use eGET to expand the PIN macro for GET macros above
#define eGET_REG(REG, PIN) GET_REG(REG, PIN)
#define eGET_REG_BIT0(REG, PIN) GET_REG_BIT0(REG, PIN)
#define eGET_REG_BIT1(REG, PIN) GET_REG_BIT1(REG, PIN)
#define eGET_AFRL_REG(REG, PIN) GET_AFRL_REG(REG, PIN)

#endif /* REG_MACROS_H */
