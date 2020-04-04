# Coding Style

## Include Order

Includes should be done in this order:

```
#include <stdio.h>

#include "example.h"
```

## If Statements

If statements should be done like this:

```
if(a == 10)
{
  return 10;
}
```

## Type Definitions

Type definitions should be done with capital letters.

```
typedef struct Example {
  int amount;
} Example;

Example example;
example.amount = 5;
```

# Enum Values

Enum Values should include a capitalised version of the type as the prefix.

```
typedef enum {
  STATUS_SUCCESS,
  STATUS_FAILURE,
} Status;

Status status = STATUS_FAILURE;
```
