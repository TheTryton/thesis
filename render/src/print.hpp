#pragma once

#include <helpers.hpp>

template<typename Dispatch>
void printLayersWithInstanceExtensions(const Dispatch& dispatchLoader)
{
    const auto layers = vk::enumerateInstanceLayerProperties(dispatchLoader);

    cout << "Layers:" << endl;
    for(const auto& layerProperties : layers)
    {
        cout << "\t- " << layerProperties.layerName.data() << ":" << endl;
        cout << "\t\t- description = " << layerProperties.description.data() << endl;
        cout << "\t\t- implementation version = " << version_wrapper_t{layerProperties.implementationVersion} << endl;
        cout << "\t\t- specification version = " << version_wrapper_t{layerProperties.specVersion} << endl;
        cout << "\t\t- extensions:" << endl;
        const auto extensions = vk::enumerateInstanceExtensionProperties(std::string(layerProperties.layerName.data()), dispatchLoader);
        for(const auto& extension : extensions)
        {
            cout << "\t\t\t- " << extension.extensionName.data() << '[' << version_wrapper_t{extension.specVersion} << ']' << endl;
        }
    }
}

template<typename Dispatch>
void printInstanceExtensions(const Dispatch& dispatchLoader)
{
    const auto extensions = vk::enumerateInstanceExtensionProperties(std::nullptr_t{}, dispatchLoader);

    cout << "extensions:" << endl;
    for(const auto& extension : extensions)
    {
        cout << "\t- " << extension.extensionName.data() << '[' << version_wrapper_t{extension.specVersion} << ']' << endl;
    }
}

template<typename Dispatch>
void printAllPhysicalDevicesWithExtensionsAndProperties(const vk::Instance& instance, const Dispatch& dispatchLoader)
{
    cout << "Physical devices:" << endl;
    const auto physicalDevices = instance.enumeratePhysicalDevices(dispatchLoader);
    for(const auto& physicalDevice : physicalDevices)
    {
        const auto physicalDeviceProperties = physicalDevice.getProperties(dispatchLoader);
        const auto physicalDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties(nullptr_t{}, dispatchLoader);

        cout << "\t- " << physicalDeviceProperties.deviceName.data() << endl;
        cout << "\t\t- api version =\t\t" << version_wrapper_t{physicalDeviceProperties.apiVersion} << endl;
        cout << "\t\t- driver version =\t" << version_wrapper_t{physicalDeviceProperties.driverVersion} << endl;
        cout << "\t\t- device id =\t\t" << physicalDeviceProperties.deviceID << endl;
        cout << "\t\t- vendor id =\t\t" << physicalDeviceProperties.vendorID << endl;
        cout << "\t\t- extensions:" << endl;
        for(const auto& physicalDeviceExtension : physicalDeviceExtensions)
        {
            cout << "\t\t\t- " << physicalDeviceExtension.extensionName.data() << '[' << version_wrapper_t{physicalDeviceExtension.specVersion} << ']' << endl;
        }
    }
}
