#include <iostream>
#include <memory>
#include <verilated.h>
#include <verilated_vpi.h>

#include "Vone.h"


namespace {

  t_vpi_time vt{ vpiSimTime, 0, 0, 0.0 };
  vluint64_t tickcount = 0;


  PLI_INT32 vchgcb(p_cb_data data)
  {
    if (data->time != nullptr) {
      switch (data->time->type) {
      case vpiScaledRealTime:
        std::cout << "t=" << data->time->real << ": ";
        break;
      case vpiSimTime:
        std::cout << "t=" << ((vluint64_t(data->time->high) << 32) | data->time->low) << ": ";
        break;
      case vpiSuppressTime:
        std::cout << "t=suppress: ";
        break;
      }
    }
    std::cout << "data->value=" << data->value << std::endl;
    switch (data->value->format) {
    case vpiScalarVal:
      std::cout << "new value: " << data->value->value.scalar << std::endl;
      break;
    case vpiIntVal:
      std::cout << "new value: " << data->value->value.integer << std::endl;
      break;
    default:
      std::cout << "unknown format " << data->value->format << " in new value\n";
    }
    return 0;
  }
}


double sc_time_stamp()
{
  return tickcount / 2.0;
}


void dump_modules(vpiHandle base)
{
  auto modit = vpi_iterate(vpiModule, base);
  if (modit) {
    vpiHandle modh;
    while ((modh = vpi_scan(modit))) {
      auto s = vpi_get_str(vpiName, modh);
      std::cout << "modh name = " << s << std::endl;

      dump_modules(modh);
    }
  }

  auto varit = vpi_iterate(vpiReg, base);
  if (varit) {
    vpiHandle varh;
    while ((varh = vpi_scan(varit))) {
      auto s = vpi_get_str(vpiName, varh);
      std::cout << "varh name = " << s << std::endl;
    }
  }
}


int main(int argc, char* argv[])
{
  Verilated::debug(0);
  Verilated::randReset(42);
  Verilated::commandArgs(argc, argv);

  auto top = std::make_unique<Vone>();

  auto onestate = vpi_handle_by_name((PLI_BYTE8*)"TOP.one.state", nullptr);
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

  Verilated::internalsDump();
  dump_modules(nullptr);

  VerilatedVpi vpi;

  while (! Verilated::gotFinish()) {
    top->clk = ! top->clk;
    top->eval();
    vpi.callValueCbs();
    ++tickcount;
    vt.high = (tickcount / 2) >> 32;
    vt.low = (tickcount / 2) & 0xffffffff;
  }
}
