#include <stdio.h>

#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#include "prussdrv.h"
#include <pruss_intc_mapping.h>

#define AM33XX

#define PRU_NUM0 	0

#define DEBUG

#define UIO_PRUSS_SYSFS_BASE				"/sys/class/uio/uio0/maps/map1"
#define UIO_PRUSS_DRAM_SIZE			        UIO_PRUSS_SYSFS_BASE "/size"
#define UIO_PRUSS_DRAM_ADDR			        UIO_PRUSS_SYSFS_BASE "/addr"

#define PRU_PAGE_SIZE 2048

#define ALIGN_TO_PAGE_SIZE(x, pagesize)  ((x)-((x)%pagesize))

typedef struct {
    uint32_t    run_flag;
    uint32_t    ddr_base_address;
    uint32_t    ddr_bytes_available;
} pru_params;

typedef struct {
    uint32_t ddr_size;
    uint32_t ddr_base_address;

	void *ddr_memory;
	void *pru_memory;

    pru_params* pru_params;

	int mem_fd;

} app_data;

void sleepms(int ms) {
	nanosleep((struct timespec[]){{0, ms*100000}}, NULL);
}

static uint32_t read_uint32_hex_from_file(const char *file) {
	size_t len = 0;
	ssize_t bytes_read;
	char *line;
	uint32_t value = 0;
    FILE *f = fopen(file, "r");
    if (f) {
		bytes_read = getline(&line, &len, f);
		if (bytes_read > 0) {
			value = strtoul(line, NULL, 0);
		}
    }
	if (f) fclose(f);
	if (line) free(line);
	return value;
}

static int load_pruss_dram_info(app_data *info) {

	info->ddr_size = read_uint32_hex_from_file(UIO_PRUSS_DRAM_SIZE);
	info->ddr_base_address = read_uint32_hex_from_file(UIO_PRUSS_DRAM_ADDR);

	return 0;
}

static int init(app_data *info)
{

	load_pruss_dram_info(info);

    info->mem_fd = open("/dev/mem", O_RDWR);
    if (info->mem_fd < 0) {
        printf("Failed to open /dev/mem (%s)\n", strerror(errno));
        return -1;
    }

    info->ddr_memory = mmap(0, info->ddr_size, PROT_WRITE | PROT_READ, MAP_SHARED, info->mem_fd, info->ddr_base_address);
    if (info->ddr_memory == NULL) {
        printf("Failed to map the device (%s)\n", strerror(errno));
        close(info->mem_fd);
        return -1;
    }

    prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void *) &info->pru_memory);

	if (info->pru_memory == NULL) {
	    fprintf(stderr, "Cannot map PRU0 memory buffer.\n");
	    return -ENOMEM;
	}

    info->pru_params = info->pru_memory;

    fprintf(stderr, "Zeroing DDR memory\n");

	memset((void *)info->ddr_memory, 0, info->ddr_size);

    fprintf(stderr, "Writing PRU params\n");

    // Set the run flag to 1
    info->pru_params->run_flag = 1;

	// Write DRAM base addr into PRU memory
    info->pru_params->ddr_base_address = info->ddr_base_address;
	// Write # bytes available
	info->pru_params->ddr_bytes_available = info->ddr_size;

    fprintf(stderr, "Init complete\n");

    return(0);
}

void check(app_data *info) {
    int i = 0;
    uint32_t *ddr = info->ddr_memory;
    for (i = 0; i < 10; i++) {
        printf("%i: 0x%X\n", i, ddr[i]);
    }
    printf("\n");
}

app_data info;

void deinit(app_data *info) {
    prussdrv_pru_disable(PRU_NUM0);
    prussdrv_exit();
    munmap(info->ddr_memory, info->ddr_size);
	close(info->mem_fd);
}


void intHandler(int dummy) {
	// Set the run flag to 0
    printf("Setting run flag to 0\n");
    info.pru_params->run_flag = 0;
}

int workthread_running = 1;

void* work_thread(void *arg) {
    while(info.pru_params->run_flag) {
        sleepms(500);
        // Do work in this thread
    }
    workthread_running = 0;
    return NULL;
}

int main (void)
{
    unsigned int ret;
	pthread_t tid;
    // struct pru_data	pru;ls

    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    /* Initialize the PRU */
    prussdrv_init();

    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }

    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    init(&info);

	signal (SIGQUIT, intHandler);
	signal (SIGINT, intHandler);

    // Start a thread to do some work
    pthread_create(&tid, NULL, &work_thread, NULL);

    /* Execute example on PRU */
    prussdrv_exec_program (PRU_NUM0, "./ddr_access.bin");

    printf("Press ctrl-c to stop running\n\n");
    while(workthread_running) {
        sleepms(250);
    }

    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
	prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tINFO: PRU completed transfer.\r\n");
	prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

    check(&info);

    printf("deinit\r\n");
    deinit(&info);

    return(0);
}

