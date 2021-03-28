#include "decode.h"

int check_cond(int cond, arm_state *state);

int execute_dp(arm_state *state);
int execute_mult(arm_state *state);
int execute_sdt(arm_state *state);
int naive_branch(arm_state *state);
int execute_branch(arm_state *state);
int execute_b(arm_state *state);
int execute(arm_state *state);
