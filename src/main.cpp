#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>

// Include NatNet SDK
#include "NatNetTypes.h"
#include "NatNetClient.h"
#include <NatNetCAPI.h>

// Global flag to control the main loop
std::atomic<bool> g_running(true);

// Signal handler for clean termination
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    g_running = false;
}

// Callback function that receives data from the NatNet client
void NATNET_CALLCONV DataHandler(sFrameOfMocapData* data, void* pUserData)
{
    // Cast user data to our context
    NatNetClient* pClient = (NatNetClient*)pUserData;
    
    // Print frame number
    std::cout << "Frame #: " << data->iFrame << std::endl;

    // Process markers (if any)
    std::cout << "Markers: " << data->nLabeledMarkers << std::endl;
    for (int i = 0; i < data->nLabeledMarkers; i++)
    {
        sMarker& marker = data->LabeledMarkers[i];
        std::cout << "  Marker " << i << " (ID: " << marker.ID << "): Position ["
                  << marker.x << ", " << marker.y << ", " << marker.z << "]" << std::endl;
    }

    // Process rigid bodies (if any)
    std::cout << "Rigid Bodies: " << data->nRigidBodies << std::endl;
    for (int i = 0; i < data->nRigidBodies; i++)
    {
        sRigidBodyData& rb = data->RigidBodies[i];
        std::cout << "  RigidBody " << i << " (ID: " << rb.ID << "): Position ["
                  << rb.x << ", " << rb.y << ", " << rb.z << "]" << std::endl;
    }

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
    // Register signal handler for clean termination
    signal(SIGINT, signalHandler);
    
    // Create NatNet client
    NatNetClient* client = new NatNetClient();
    
    // Set callback handlers
    client->SetFrameReceivedCallback(DataHandler, client);
    
    // Connect to Motive
    std::string serverIP = "127.0.0.1"; // IP address of the Motive/server machine
    std::string localIP = "127.0.0.1";  // IP address of this machine
    
    std::cout << "Connecting to Motive at " << serverIP << "..." << std::endl;
    
    // Initialize client with connection settings
    sNatNetClientConnectParams connectParams;
    connectParams.connectionType = ConnectionType_Multicast; // or Unicast if needed
    connectParams.serverCommandPort = 1510;
    connectParams.serverDataPort = 1511;
    connectParams.serverAddress = serverIP.c_str();
    connectParams.localAddress = localIP.c_str();
    ErrorCode result = client->Connect(connectParams);
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
        client->Disconnect();
        delete client;
        return 1;
    }
    
    std::cout << "Connected to Motive server: " << ServerDescription.szHostApp << std::endl;
    std::cout << "Server version: " << ServerDescription.HostAppVersion << std::endl;
    std::cout << "NatNet version: " << ServerDescription.NatNetVersion << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    
    // Main loop
    try
    {
        std::cout << "Requesting data descriptions..." << std::endl;
        sDataDescriptions* pDataDefs = NULL;
        client->GetDataDescriptionList(&pDataDefs);
        if (pDataDefs)
        {
            std::cout << "Retrieved " << pDataDefs->nDataDescriptions << " data descriptions." << std::endl;
            
            for (int i = 0; i < pDataDefs->nDataDescriptions; i++)
            {
                if (pDataDefs->arrDataDescriptions[i].type == Descriptor_MarkerSet)
                {
                    sMarkerSetDescription* pMS = pDataDefs->arrDataDescriptions[i].Data.MarkerSetDescription;
                    std::cout << "MarkerSet: " << pMS->szName << " (" << pMS->nMarkers << " markers)" << std::endl;
                }
                else if (pDataDefs->arrDataDescriptions[i].type == Descriptor_RigidBody)
                {
                    sRigidBodyDescription* pRB = pDataDefs->arrDataDescriptions[i].Data.RigidBodyDescription;
                    std::cout << "RigidBody: " << pRB->szName << " (ID: " << pRB->ID << ")" << std::endl;
                }
            }
            
            NatNet_FreeDescriptions(pDataDefs);
        }
        else
        {
            std::cout << "Unable to retrieve data descriptions." << std::endl;
        }
        
        std::cout << "\nStarting main loop. Data will be printed as it arrives." << std::endl;
        std::cout << "Press Ctrl+C to exit cleanly.\n" << std::endl;
        
        while (g_running)
        {
            client->GetDataDescriptionList(&pDataDefs);
            
            
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
    client->Disconnect();
    delete client;
    
    return 0;
}

