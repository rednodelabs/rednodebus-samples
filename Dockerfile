FROM coderbyheart/fw-nrfconnect-nrf-docker:latest
RUN rm -rf /workdir/rnl
RUN mkdir -p /workdir/rnl/rednodebus
COPY ./west.yml /workdir/rnl/rednodebus
RUN \
    # Zephyr requirements of rednodebus
    cd /workdir/rnl/rednodebus; west init -l && \
    cd /workdir/rnl; west update && \
    pip3 install -r zephyr/scripts/requirements.txt && \
    pip3 install -r nrf/scripts/requirements.txt && \
    pip3 install -r bootloader/mcuboot/scripts/requirements.txt && \
    rm -rf /workdir/rnl/rednodebus
