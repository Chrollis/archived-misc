#pragma once

namespace server {

enum class job_state {
    idle,
    running,
    cancelling,
    finished,
    cancelled,
    error,
};

inline const char* to_string(job_state state) {
    switch (state) {
        case job_state::idle:
            return "idle";
        case job_state::running:
            return "running";
        case job_state::cancelling:
            return "cancelling";
        case job_state::finished:
            return "finished";
        case job_state::cancelled:
            return "cancelled";
        case job_state::error:
            return "error";
    }
    return "unknown";
}

}  // namespace server
