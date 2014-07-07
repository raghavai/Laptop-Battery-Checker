#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h> 
#include <linux/slab.h>
#include <acpi/acpi.h>
#include <asm/uaccess.h>
#include <asm/i387.h>
#include <acpi/actypes.h>

MODULE_LICENSE("GPL");

struct workqueue_struct *queue;
struct delayed_work *dwork;
acpi_status status;
acpi_handle handle;
union acpi_object *bif_result, *bst_result;
int power_unit,last_full_cap,bat_low,bat_state, bat_remain_cap, bat_present_vol, bat_present_rate;
char unit[10];
int x;
int delay = 30;
//int count = 0;
struct acpi_buffer bst_buffer = { ACPI_ALLOCATE_BUFFER, NULL };
struct acpi_buffer bif_buffer = { ACPI_ALLOCATE_BUFFER, NULL };

int battcheck(struct work_struct *work) {
	//printk(KERN_EMERG "Count is %d",count++);
	status = acpi_get_handle(NULL, "\\_SB.PCI0.BAT0", &handle);
	status = acpi_evaluate_object(handle, "_BIF", NULL, &bif_buffer);
	status = acpi_evaluate_object(handle, "_BST", NULL, &bst_buffer);

	if(ACPI_FAILURE(status)) {
		printk(KERN_ERR "acpi_call:cannot get handle");
	}
	
	bif_result = bif_buffer.pointer;
	if(bif_result) {

		power_unit = bif_result->package.elements[0].integer.value;
		if(power_unit == 1) strcpy(unit,"mWh");
		else strcpy(unit,"mAh");

		last_full_cap = bif_result->package.elements[2].integer.value;
		bat_low = bif_result->package.elements[6].integer.value;
	}

	bst_result = bst_buffer.pointer;
	if(bst_result) {

		bat_remain_cap = bst_result->package.elements[2].integer.value;
	
		if(bat_remain_cap == last_full_cap) 
			printk(KERN_INFO "Battery is fully charged");

		else if(bat_remain_cap == bat_low) 
			printk(KERN_INFO "Battery is low");
		

		bat_state = bst_result->package.elements[0].integer.value;
		if(bat_state == 1) {
			kernel_fpu_begin();
			x = bat_remain_cap*100;
			x = x/last_full_cap;
			printk(KERN_INFO "Battery is discharging...%d %c battery remaining",x,0X25);
			kernel_fpu_end();
		}
		else if(bat_state == 2) {
			kernel_fpu_begin();
			x = bat_remain_cap*100;
			x = x/last_full_cap;
			printk(KERN_INFO "Battery is charging... %d %c battery available",x,0X25);
			kernel_fpu_end();
		}
		else 	{
			printk(KERN_INFO "Battery is critically low");
			queue_delayed_work(queue, dwork, delay*HZ);
			return 0;
		}
	}
	/*printk(KERN_INFO "Battery State = %d\n , Battery Remaining Capacity = %d %s\n", bat_state, 			bat_remain_cap, unit); 
	printk(KERN_INFO "Power Unit = %d\n , Last Full Charge Capacity = %d %s\n , Low Battery 			Capacity = %d %s \n ", power_unit, last_full_cap, unit, bat_low, unit); */
	queue_delayed_work(queue, dwork, HZ);
	return 0;
}

int init_module(void) {
	printk(KERN_INFO"Module is loading..");
	queue = create_workqueue("queue");
	dwork = (struct delayed_work*)kmalloc(sizeof(struct delayed_work), GFP_KERNEL);
	INIT_DELAYED_WORK((struct delayed_work*)dwork, battcheck);
	queue_delayed_work(queue, dwork, HZ);
	return 0;
}

void cleanup_module(void) {
	flush_workqueue(queue);
	if(dwork && delayed_work_pending(dwork)) {
		cancel_delayed_work(dwork);
		flush_workqueue(queue);
	}
	destroy_workqueue(queue);
	printk(KERN_EMERG"Modue unloaded successfuly");
}

