//
// Created by Yihua on 2024/2/23.
//

#include "Application.h"

Application *Application::instance() {
    static Application app;
    return &app;
}

Application::Application() {

}
