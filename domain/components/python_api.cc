#include "market/domain/components/python_api.h"
#include "common/extern/pybind11/include/pybind11/pybind11.h"

bool TradeDate::GetLoginDate(const std::string &timestring, char *login_date) {
  bool ret = true;
  pybind11::gil_scoped_acquire acquire;
  pybind11::module_ importlib = pybind11::module_::import("importlib");
  pybind11::module_ sys = pybind11::module_::import("sys");

  auto modules = {"ticknature.trade_date"};
  for (auto &module : modules) {
    if (sys.attr("modules").contains(module)) {
      importlib.attr("reload")(sys.attr("modules")[module]);
    }
  }

  pybind11::object tradedate = pybind11::module::import("ticknature.trade_date").attr("tradedate");

  pybind11::object obj = tradedate.attr("get_login_date")(timestring);
  strcpy(login_date, obj.cast<std::string>().c_str());

  return ret;
}
