#define JS_REGISTER_METHOD(c, a) registerMethod(#a, make_method(this, &c::a));
#define JS_REGISTER_RO_PROPERTY(c, a) registerProperty(#a, \
                                          make_property(this, &c::get_##a));
