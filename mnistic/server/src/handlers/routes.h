#pragma once

#include <httplib.h>

namespace server {

void register_misc_routes(httplib::Server& svr);
void register_config_routes(httplib::Server& svr);
void register_recognize_routes(httplib::Server& svr);
void register_dataset_routes(httplib::Server& svr);
void register_model_routes(httplib::Server& svr);
void register_train_routes(httplib::Server& svr);
void register_evaluate_routes(httplib::Server& svr);

void register_routes(httplib::Server& svr);

}  // namespace server
