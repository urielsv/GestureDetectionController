#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

// Include NatNet SDK
#include "NatNetTypes.h"
#include "NatNetCAPI.h"
#include "NatNetClient.h"

// Global flag to control the main loop
std::atomic<bool> g_running(true);

// Callback function that receives data from the NatNet client
void NATNET_CALLCONV DataHandler(sFrameOfMocapData* data, void* pUserData)
{
    // Cast user data to our context
    NatNetClient* pClient = (NatNetClient*)pUserData;

    // Process skeleton data
    for (int i = 0; i < data->nSkeletons; i++)
    {
        sSkeletonData& skeleton = data->Skeletons[i];
        std::cout << "Skeleton ID: " << skeleton.skeletonID << std::endl;
        
        // Process each rigid body (joint) in the skeleton
        for (int j = 0; j < skeleton.nRigidBodies; j++)
        {
            sRigidBodyData& rb = skeleton.RigidBodyData[j];
            std::cout << "  Joint " << j << ": Position [" 
                      << rb.x << ", " << rb.y << ", " << rb.z << "]" << std::endl;    
        }
    }
}

// Function to handle messages from the NatNet client
void NATNET_CALLCONV MessageHandler(Verbosity msgType, const char* msg)
{
    // Print messages according to verbosity
    switch (msgType)
    {
    case Verbosity_Debug:
        std::cout << "Debug: " << msg << std::endl;
        break;
    case Verbosity_Info:
        std::cout << "Info: " << msg << std::endl;
        break;
    case Verbosity_Warning:
        std::cout << "Warning: " << msg << std::endl;
        break;
    case Verbosity_Error:
        std::cout << "Error: " << msg << std::endl;
        break;
    default:
        std::cout << "Message: " << msg << std::endl;
    }
}

int main(int argc, char* argv[])
{
    // Create NatNet client
    NatNetClient* client = new NatNetClient();
    
    // Set callback handlers
    client->SetFrameReceivedCallback(DataHandler, client);
    client->SetMessageCallback(MessageHandler);
    
    // Connect to Motive (lab IP)
    std::string serverIP = "127.0.0.1";
    std::string localIP = "127.0.0.1";  

    std::cout << "Connecting to Motive at " << serverIP << "..." << std::endl;
    
    // Initialize client with connection settings
    ErrorCode result = client->Initialize(localIP.c_str(), serverIP.c_str());
    if (result != ErrorCode_OK)
    {
        std::cout << "Error initializing client. Error code: " << result << std::endl;
        return 1;
    }
    
    // Print client/server info
    sServerDescription ServerDescription;
    memset(&ServerDescription, 0, sizeof(ServerDescription));
    client->GetServerDescription(&ServerDescription);
    
    if (!ServerDescription.HostPresent)
    {
        std::cout << "Unable to connect to Motive server. Exiting." << std::endl;
        client->Uninitialize();
        delete client;
        return 1;
    }
    
    std::cout << "Connected to Motive server: " << ServerDescription.szHostApp << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    
    // Main loop
    try
    {
        while (g_running)
        {
            // Sleep to avoid high CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Error in main loop: " << e.what() << std::endl;
    }
    
    // Clean up
    std::cout << "Shutting down..." << std::endl;
    client->Uninitialize();
    delete client;
    
    return 0;
}

