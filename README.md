Verilator Details
=================

Using VPI
---------

To use VPI the signals that are queried must be explicitly marked
with the comment

    /* verilator public_flat_rw */

or the `verilator` command must get the option `--public-flat-rw` added
so that unconditionally all signals are marked this way.

To Do: determine performance impact of `--public-flat-rw`.  Possibly create
filter which can be used with `--pipe-filter` to add the comment only for the
signals which are requested.  This requires, though, that the simulation
binary is recompiled which requires a completely different structure for the
code.  Instead of the simulator binary bringing up the TUI as well one needs
a standalone TUI binary which then compiles and dynamically loads the simulator.

### Accessing Signals

Ever signals and other represented entity in the simluator is accessible through
a handle.  Handles can be retrieved in different ways.  If one knows the
complete name of a signal one can use:

    auto onestate = vpi_handle_by_name((PLI_BYTE8*)"TOP.one.state", nullptr);

This retrieves the handle for a signal `state` in the top module named `one`.  This
handle is constant over the lifetime of the simulator process.  Retrieving it can happen
outside the main loop.

To get the current value of the signal use

    t_vpi_value val;
    vpi_get_value(onestate, &val);

The variable `val` is filled in appropriately, with the field `format` describing how to
interpret the remaining information.

To set the signal to a new value use

    t_vpi_value val;
    val.format = ...;
    val.value.XYZ = ...;
    t_vpi_time tim;
    tim.type = ...;
    tim.XYZ = ...;
    ... = vpi_put_value(onestate, &val, &tim, 0);


### Tracing signals

Verilator supports recording wave forms, in the VCD and FST formats.  There are two problematic
limitations, though:

1.  Tracing must be enabled before the first simulation step
2.  The trace is only accessible once the simulation process finished.

If might be possible to flush the stream of the wave form data but in general (e.g., for compression)
the data in the file is not usable until tracing stops.

An alternative is to use the VPI interface to trace changes to specified signals.  If one can get a
handle of a signal it is possible to register an appropriate callback:

    t_vpi_value vv;
    vv.format = vpiIntVal;
    t_cb_data cb;
    cb.reason = cbValueChange;
    cb.cb_rtn = vchgcb;
    cb.obj = onestate;
    cb.time = &vt;
    cb.value = &vv;
    cb.index = 0;
    cb.user_data = nullptr;
    auto onestatechange = vpi_register_cb(&cb);

In this case the signal accessible through the `onestate` handle consists of two bits.  Single bits
would be accessible as `vpiScalarVal` object, in this case `vpiIntVal` is used.  An appropriate
format must be filled in the `format` field.  The other fields are not important.  The lifetime of the
object (here `vv`) does not have to extend beyond its use in the call to `vpi_register_cb`.

This is different for the object `vt` which is passed in through the `time` member of `t_cb_data`.  This
object pointer is passed as-is to the callback function as part of the structure pointed to by the
`p_cb_data` parameter.  If the object (`vt` here) is always kept up-to-date with the current time
(simulation or real-time) then the callback function does not have to access any information other than
the parameter to get all the relevant information about the reported event.

The callback function `vchgcb` is called for every change to the signal, as requested by the `cbValueChange`
value to the `reason` member variable.  Other reasons are possible.  Requesting these callbacks is likely
much more efficient to check signals for specific value than using `vpi_get_value` after every simulation
step.  The less frequently the signal is updated the larger the benefit and vice versa.

To get the notification the main simulation loop must request the callbacks to be called.

    VerilatedVpi vpi;

    while (! Verilated::gotFinish()) {
      top->clk = ! top->clk;
      top->eval();
      vpi.callValueCbs();
      ...
    }

The variable `vpi` should be defined outside the loop.  It has static member functions
like `callValueCbs`.  This is the specific function which can be used to invoke callbacks which
signal value changes.  One could also use `callCbs` which would call any callback that has its
requirements met (e.g., time-based ones).  Using the specific function like `callValueCbs` is more
efficient.
