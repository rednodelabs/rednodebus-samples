# RedNodeLabs Device Provisioning

Our device provisioning process has been designed and implemented following the white paper of [AWS IoT Core](https://d1.awsstatic.com/whitepapers/device-manufacturing-provisioning.pdf). It is based on X.509 certificates and uses AWS IoT Core for provisioning the unique device certificates. The provisioning process is also used by RedNodeLabs to invoice the purchasing of the license.

In the certificates folder of the RNB OTBR Docker (e.g. `/home/pi/rnl_certs`) unique device certificates are stored for the border router and the nodes. Only the first time the border router or a new node is deployed, internet connection is required to purchase and download the new certificate. Once the certificates are present in the folder, internet connection is no longer required to run the system.

In the nodes, certificates are stored in flash the first time they are provisioned. If the flash is erased afterwards, the certificate is requested again to the border router when the node is connected. If it is present in the local folder, it is sent offline to the node. If it is deleted from the folder, internet access is required again to download it. If that node was already licensed, no additional license is used, i.e. the certificate for an already licensed node can be downloaded ad many times as needed without additional purchases.

To acquire a licenses account, contact [RedNodeLabs](mailto:info@rednodelabs.com).
