//
// Created by Yihua Liu on 2024/3/31.
//

#ifndef SUISAPP_MATERIAL_SYSTEM_H
#define SUISAPP_MATERIAL_SYSTEM_H

class MaterialSystem {
public:
    static MaterialSystem *instance();
private:
    MaterialSystem();
    ~MaterialSystem() = default;

    static MaterialSystem *m_instance;
};


#endif  // SUISAPP_MATERIAL_SYSTEM_H
