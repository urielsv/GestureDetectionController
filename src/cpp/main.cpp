// src/main.cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>

#include "NatNetTypes.h"
#include "NatNetCAPI.h"
#include "NatNetClient.h"

static std::atomic<bool> g_running{true};
static NatNetClient* g_client = nullptr;

void __cdecl DataHandler(sFrameOfMocapData* data, void*)
{
    if(!data) return;

    std::cout << "[Frame] " << data->iFrame
              << "  RBs: " << data->nRigidBodies
              << "  Unlabeled: " << data->nOtherMarkers
              << "  Labeled: " << data->nLabeledMarkers
              << "\n";
    /*
    for(int i=0; i<data->nRigidBodies; ++i)
    {
        const sRigidBodyData& rb = data->RigidBodies[i];
        std::cout << "  RB " << rb.ID
                  << "  pos=(" << rb.x << ", " << rb.y << ", " << rb.z << ")"
                  << "  rot(q)=(" << rb.qx << ", " << rb.qy << ", "
                  << rb.qz << ", " << rb.qw << ")\n";
    }*/

 for(int i=0; i<data->nOtherMarkers; ++i) {
        const MarkerData& m = data->OtherMarkers[i];
        //std::cout << "  UL[" << i << "] pos=(" << m.x << ", " << m.y << ", " << m.z << ")\n";
    }

}

void __cdecl MessageHandler(int msgType, char* szMsg)
{
    std::cout << "[NatNet] (" << msgType << ") " << (szMsg ? szMsg : "") << "\n";
}

int main(int argc, char** argv)
{
    // Parámetros (podés sobreescribir por CLI)
    // serverAddress: IP de la PC con Motive
    // localAddress:  IP de la PC que corre este cliente
    const char* serverAddress   = "10.17.2.137"; // <- cambiala si tu Motive usa otra
    const char* localAddress    = "10.17.2.137"; // <- si corres en la misma PC
    const char* multicastAddr   = "224.0.0.1";
    int         commandPort     = 1510;
    int         dataPort        = 1511;
    ConnectionType connType     = ConnectionType_Multicast;

    if(argc >= 2) serverAddress = argv[1];
    if(argc >= 3) localAddress  = argv[2];

    std::cout << "Server(Motive): " << serverAddress << "\n";
    std::cout << "Local Client  : " << localAddress  << "\n";
    std::cout << "Multicast     : " << multicastAddr << "\n";
    std::cout << "Cmd/Data Ports: " << commandPort << "/" << dataPort << "\n";

    // Crear cliente
    g_client = new NatNetClient();

    // Callback de mensajes (log interno NatNet)
    g_client->SetMessageCallback(MessageHandler);

    // Callback de datos por frame
    sNatNetClientConnectParams params;
    memset(&params, 0, sizeof(params));
    params.connectionType     = connType;                 // Multicast
    params.serverAddress      = serverAddress;
    params.localAddress       = localAddress;
    params.multicastAddress   = multicastAddr;
    params.serverCommandPort  = commandPort;
    params.serverDataPort     = dataPort;

    int rc = g_client->Connect(params);
    if(rc != ErrorCode_OK)
    {
        std::cerr << "ERROR: NatNet Connect() fallo con codigo " << rc << "\n";
        return 1;
    }

    // Registrar callback de data (se llama en cada frame)
    rc = g_client->SetDataCallback(DataHandler, nullptr);
    if(rc != ErrorCode_OK)
    {
        std::cerr << "ERROR: SetDataCallback() codigo " << rc << "\n";
        g_client->Disconnect();
        return 1;
    }

    // (Opcional) pedir descripciones de la escena una vez
    sDataDescriptions* pDesc = nullptr;
    rc = g_client->GetDataDescriptions(&pDesc);
    if(rc == ErrorCode_OK && pDesc)
    {
        std::cout << "Scene descriptions: " << pDesc->nDataDescriptions << "\n";
        // Podrías iterar y mapear nombres/IDs aquí.
    }
    else
    {
        std::cout << "No se pudieron obtener descripciones (rc=" << rc << ")\n";
    }

    std::cout << "Recibiendo datos... (Ctrl+C para salir)\n";

    // Loop simple para mantener vivo el cliente
    while(g_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    g_client->Disconnect();
    delete g_client;
    return 0;
}
