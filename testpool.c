	// Starts de pool
    
	printf("vai passar\n");
	threadpool myThreadPool = create_threadpool(10);
	do_work(&myThreadPool);
	printf("passou\n");
	int (*_getComponent)(int) = &getComponent;
	dispatch(myThreadPool, (dispatch_fn)_getComponent, &c);
	