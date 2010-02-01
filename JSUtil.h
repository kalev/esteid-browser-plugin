#define RTERROR_TO_SCRIPT(exp) try { exp; } \
    catch(std::runtime_error &e) { throw FB::script_error(e.what()); }

#define JS_REGISTER_METHOD(c, a) registerMethod(#a, make_method(this, &c::a));
#define JS_REGISTER_RO_PROPERTY(c, a) registerProperty(#a, \
                                          make_property(this, &c::get_##a));
