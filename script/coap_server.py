import datetime
import logging
import asyncio
import aiocoap.resource as resource
import aiocoap

THREAD_COAP_UTILS_LIGHT_CMD_TOGGLE = b'2'

class LightResource(resource.Resource):
    """Example light resource which supports the PUT method."""

    def __init__(self):
        self.light_state = True

    def toggle_light(self):
        self.light_state = not self.light_state

    async def render_put(self, request):
        print('Received PUT method in light resource')
        print('PUT payload: %s' % request.payload)
        
        if request.payload == THREAD_COAP_UTILS_LIGHT_CMD_TOGGLE:
            self.toggle_light()
            print("Light toggled: "+str(self.light_state))            
        
        return aiocoap.Message(code=aiocoap.CHANGED, payload=bytes(self.light_state))

# logging setup

logging.basicConfig(level=logging.INFO)
logging.getLogger("coap-server").setLevel(logging.DEBUG)

async def main():
    # Resource tree creation
    root = resource.Site()

    root.add_resource(['.well-known', 'core'],
            resource.WKCResource(root.get_resources_as_linkheader))
    root.add_resource(['light'], LightResource())

    await aiocoap.Context.create_server_context(root)

    # Run forever
    await asyncio.get_running_loop().create_future()

if __name__ == "__main__":
    asyncio.run(main())