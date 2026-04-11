// QuantumFractalSystem — Entry point

#include "app/Application.h"

int main() {
    qfs::Application app;
    app.init();
    app.run();
    app.shutdown();
    return 0;
}
