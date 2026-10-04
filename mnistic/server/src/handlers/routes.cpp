#include <handlers/routes.h>

namespace server {

void register_routes(httplib::Server& svr) {
    register_misc_routes(svr);
    register_config_routes(svr);
    register_recognize_routes(svr);
    register_dataset_routes(svr);
    register_model_routes(svr);
    register_train_routes(svr);
    register_evaluate_routes(svr);
}

}  // namespace server
