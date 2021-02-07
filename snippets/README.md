# Snippets

## Zephyr RTOS

### Print name of threads
Set CONFIG_THREAD_NAME=y in prj.conf

```C    
void thread_printer(const struct k_thread *thread, void *user_data)
{
	ARG_UNUSED(user_data);
	printk("THREAD: %s\r\n", thread->name);
}
void main(void)
{
    k_thread_foreach(thread_printer, NULL);
}
```

### Debugging

CONFIG_EXTRA_EXCEPTION_INFO
