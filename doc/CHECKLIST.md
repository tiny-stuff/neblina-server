Every source file:
- [ ] Are structs hidden?
- [ ] Is it following module_action.c standard?
- [ ] Are functions being exported in .h and _priv.h
- [ ] Does it have unit tests?
- [ ] Does it pass without warnings?

Every function:
- [ ] Is it following module_class_method name?
- [ ] Is using OO when it makes sense?
- [ ] Is there custom OS code that needs to be isolated?
- [ ] Is it catching all possible failures?
- [ ] Is it retuning <0 in case of errors?
