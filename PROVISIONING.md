# RedNodeLabs Device Provisioning

Our device provisioning process has been designed and implemented following the white paper of [AWS IoT Core](https://d1.awsstatic.com/whitepapers/device-manufacturing-provisioning.pdf). It is based on X.509 certificates and uses AWS IoT Core for provisioning the unique device certificates. The provisioning process is also used by RedNodeLabs to invoice the purchasing of the license.

In the certificates folder (i.e. `/home/pi/rnl_certs`) unique device certificates are stored for the border router and the nodes. Only the first time the border router or a new node is deployed, internet connection is required to download the new certificate. Once the certificates are present in the folder, internet connection is no longer required to run the system.