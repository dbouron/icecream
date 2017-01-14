/* -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 99; -*- */
/* vim: set ts=4 sw=4 et tw=99:  */
// code based on gcc - Copyright (C) 1999, 2000, 2001, 2002 Free Software Foundation, Inc.

#ifndef GCC_H
# define GCC_H

# include <algorithm>

int ggc_min_expand_heuristic(unsigned int mem_limit);
unsigned int ggc_min_heapsize_heuristic(unsigned int mem_limit);

#endif /* !GCC_H */
