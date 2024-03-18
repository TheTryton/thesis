#pragma once

#include <helpers.hpp>

template<typename Dispatch>
void printLayersWithInstanceExtensions(const Dispatch& dispatchLoader)
{
    const auto layers = vk::enumerateInstanceLayerProperties(dispatchLoader);

    cout << format("Layers:") << endl;
    for(const auto& layerProperties : layers)
    {
        cout << format("\t- {}:", layerProperties.layerName.data()) << endl;
        cout << format("\t\t- description = {}", layerProperties.description.data()) << endl;
        cout << format("\t\t- implementation version = {}", version_wrapper_t{layerProperties.implementationVersion}) << endl;
        cout << format("\t\t- specification version = {}", version_wrapper_t{layerProperties.specVersion}) << endl;
        cout << format("\t\t- extensions:") << endl;
        const auto extensions = vk::enumerateInstanceExtensionProperties(std::string(layerProperties.layerName.data()), dispatchLoader);
        for(const auto& extension : extensions)
        {
            cout << format("\t\t\t- {} [{}]", extension.extensionName.data(), version_wrapper_t{extension.specVersion}) << endl;
        }
    }
}

template<typename Dispatch>
void printInstanceExtensions(const Dispatch& dispatchLoader)
{
    const auto extensions = vk::enumerateInstanceExtensionProperties(std::nullptr_t{}, dispatchLoader);

    cout << format("extensions:") << endl;
    for(const auto& extension : extensions)
    {
        cout << format("\t- {} [{}]", extension.extensionName.data(), version_wrapper_t{extension.specVersion}) << endl;
    }
}

template<typename Dispatch>
void printAllPhysicalDevicesWithExtensionsAndProperties(const vk::Instance& instance, const Dispatch& dispatchLoader)
{
    cout << format("Physical devices:") << endl;
    const auto physicalDevices = instance.enumeratePhysicalDevices(dispatchLoader);
    for(const auto& physicalDevice : physicalDevices)
    {
        const auto physicalDeviceProperties = physicalDevice.getProperties(dispatchLoader);
        const auto physicalDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties(nullptr_t{}, dispatchLoader);

        cout << format("\t- {}", physicalDeviceProperties.deviceName.data()) << endl;
        cout << format("\t\t- api version =\t\t{}", version_wrapper_t{physicalDeviceProperties.apiVersion}) << endl;
        cout << format("\t\t- driver version =\t{}", version_wrapper_t{physicalDeviceProperties.driverVersion}) << endl;
        cout << format("\t\t- device id =\t\t{}", physicalDeviceProperties.deviceID) << endl;
        cout << format("\t\t- vendor id =\t\t{}", physicalDeviceProperties.vendorID) << endl;
        cout << format("\t\t- extensions:") << endl;
        for(const auto& physicalDeviceExtension : physicalDeviceExtensions)
        {
            cout << format("\t\t\t- {} [{}]", physicalDeviceExtension.extensionName.data(), version_wrapper_t{physicalDeviceExtension.specVersion}) << endl;
        }
    }
}
