 #include <linux/kernel.h>
 #include <linux/slab.h>
 #include <linux/module.h>
 #include <linux/init.h>
 #include <linux/usb/input.h>
 #include <linux/hid.h>

 #define DRIVER_VERSION "v1.6"
 #define DRIVER_AUTHOR "Vojtech Pavlik <vojtech@ucw.cz>"
 #define DRIVER_DESC "USB HID Boot Protocol gamepad driver"
 #define DRIVER_LICENSE "GPL"

MODULE_AUTHOR(DRIVER_AUTHOR);
 MODULE_DESCRIPTION(DRIVER_DESC);
  MODULE_LICENSE(DRIVER_LICENSE);
   static const char keycode[11]={KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_ESC,KEY_ENTER,KEY_TAB,KEY_HOME,KEY_PAGEDOWN,KEY_PAGEUP,KEY_LEFTSHIFT};
struct usb_gamepad {
	  char name[128];
	    char phys[64];
	      struct usb_device *usbdev;
	        struct input_dev *dev;
		  struct urb *irq;
		    signed char *data;
		      dma_addr_t data_dma;
		       
};

static void usb_gamepad_irq(struct urb *urb)
{
	  struct usb_gamepad *gamepad = urb->context;
	    signed char *data = gamepad->data;
	      struct input_dev *dev = gamepad->dev;
	        int status;

		switch (urb->status) {
			  case 0:   
				     break;
				       case -ECONNRESET: 
				       case -ENOENT:
				       case -ESHUTDOWN:
				        return;
					  
					  default:  
					   goto resubmit;
					     
		}
		  input_report_key(dev, KEY_UP,    (data[5] & 0x10)||(!(data[4]&0x10)));
		    input_report_key(dev, KEY_LEFT,  (!(data[3] & 0x10))||(data[5]&0x80));
		      input_report_key(dev, KEY_DOWN,     (data[4] & 0x80)||(data[5]&0x40));
		        input_report_key(dev, KEY_RIGHT,   (data[3] & 0x80)||(data[5]&0x20));
			  input_report_key(dev, KEY_ESC,  data[6] & 0x10);
			    input_report_key(dev, KEY_ENTER,  data[6] & 0x22);
			      input_report_key(dev, KEY_TAB,  data[6] & 0x01);
			        input_report_key(dev, KEY_HOME,  data[6] & 0x04);
				  input_report_key(dev, KEY_PAGEDOWN,  data[6] & 0x10);
				    input_report_key(dev, KEY_PAGEUP,  data[6] & 0x80);
				      input_report_key(dev, KEY_LEFTSHIFT,  data[6] & 0x20);
				        input_sync(dev);
					 resubmit:
					  status = usb_submit_urb (urb, GFP_ATOMIC);
					   // if (status)
						      // err ("can't resubmit intr, %s-%s/input0, status %d",
							//	            gamepad->usbdev->bus->bus_name,
							//		         gamepad->usbdev->devpath, status);
					     
}

static int usb_gamepad_open(struct input_dev *dev)
{
	  struct usb_gamepad *gamepad = input_get_drvdata(dev);

	   gamepad->irq->dev = gamepad->usbdev;
	     if (usb_submit_urb(gamepad->irq, GFP_KERNEL))
		        return -EIO;

	      return 0;
	       
}

static void usb_gamepad_close(struct input_dev *dev)
{
	  struct usb_gamepad *gamepad = input_get_drvdata(dev);

	   usb_kill_urb(gamepad->irq);
	    
}

static int usb_gamepad_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	  
	  
	  struct usb_device *dev = interface_to_usbdev(intf);
	    int i;
	      
	      struct usb_host_interface *interface; 
	        
	        struct usb_endpoint_descriptor *endpoint; 
		  struct usb_gamepad *gamepad;
		    struct input_dev *input_dev;
		      int pipe, maxp;
		        int error = -ENOMEM;

			 
			  interface = intf->cur_altsetting;

			    
			    if (interface->desc.bNumEndpoints != 1)
				       return -ENODEV;
			      
			      endpoint = &interface->endpoint[0].desc; 
			        
			        if (!usb_endpoint_is_int_in(endpoint))
					   return -ENODEV;
				  
				  pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress); 
				     
				    maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));
				      
				      gamepad = kzalloc(sizeof(struct usb_gamepad), GFP_KERNEL);
				        input_dev = input_allocate_device();
					  if (!gamepad || !input_dev)
						     goto fail1;

					    
					    gamepad->data = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &gamepad->data_dma);
					      if (!gamepad->data)
						         goto fail1;
					        
					        
					        gamepad->irq = usb_alloc_urb(0, GFP_KERNEL);
						  if (!gamepad->irq)
							     goto fail2;
						    gamepad->usbdev = dev;
						      gamepad->dev = input_dev;

						       if (dev->manufacturer)
							          strlcpy(gamepad->name, dev->manufacturer, sizeof(gamepad->name));

						       if (dev->product) {
							          if (dev->manufacturer)
									      strlcat(gamepad->name, " ", sizeof(gamepad->name));
								     strlcat(gamepad->name, dev->product, sizeof(gamepad->name));
								       
						       }

						        if (!strlen(gamepad->name))
								   snprintf(gamepad->name, sizeof(gamepad->name),
										        "USB HIDBP gamepad %x:%x",
											     le16_to_cpu(dev->descriptor.idVendor),
											          le16_to_cpu(dev->descriptor.idProduct));

							 usb_make_path(dev, gamepad->phys, sizeof(gamepad->phys));//获取USB设备在sysfs中的位置
							  strlcat(gamepad->phys, "/input0", sizeof(gamepad->phys));

							   input_dev->name = gamepad->name;
							     input_dev->phys = gamepad->phys;
							       usb_to_input_id(dev, &input_dev->id);
							         input_dev->dev.parent = &intf->dev;

								  input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
								    for(i=0;i<11;i++)
									       set_bit(keycode[i],input_dev->keybit);
								      input_set_drvdata(input_dev, gamepad);
								        input_dev->open = usb_gamepad_open;
									  input_dev->close = usb_gamepad_close;

									   
									    usb_fill_int_urb(gamepad->irq, dev, pipe, gamepad->data,
											         (maxp > 8 ? 8 : maxp),
												      usb_gamepad_irq, gamepad, endpoint->bInterval);
									      gamepad->irq->transfer_dma = gamepad->data_dma;
									        gamepad->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

										 error = input_register_device(gamepad->dev);
										   if (error)
											      goto fail3;

										    usb_set_intfdata(intf, gamepad);
										      return 0;

fail3: 
										        usb_free_urb(gamepad->irq);
											 fail2: 
											  usb_free_coherent(dev, 8, gamepad->data, gamepad->data_dma);
											   fail1: 
											    input_free_device(input_dev);
											      kfree(gamepad);
											        return error;
												 
}

static void usb_gamepad_disconnect(struct usb_interface *intf)
{
	  struct usb_gamepad *gamepad = usb_get_intfdata (intf);

	   usb_set_intfdata(intf, NULL);
	   if (gamepad) {
		      usb_kill_urb(gamepad->irq);
		         input_unregister_device(gamepad->dev);
			    usb_free_urb(gamepad->irq);
			       usb_free_coherent(interface_to_usbdev(intf), 8, gamepad->data, gamepad->data_dma);
			          kfree(gamepad);
				    
	   }
	    
}

static struct usb_device_id usb_gamepad_id_table [] = {
	  { USB_DEVICE(0x0079,0x0011) },
	  { USB_DEVICE(0x0003,0x0002) },
	    {  } 
	   
};

MODULE_DEVICE_TABLE (usb, usb_gamepad_id_table);

static struct usb_driver usb_gamepad_driver = {
	  .name  = "usbgamepad",  
	    .probe  = usb_gamepad_probe,
	      .disconnect = usb_gamepad_disconnect,
	        .id_table = usb_gamepad_id_table,
		 
};

static int __init usb_gamepad_init(void)
{
	  
	  int retval = usb_register(&usb_gamepad_driver);
	    if (retval == 0)
		       printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
				            DRIVER_DESC "\n");
	      return retval;
	       
}
 static void __exit usb_gamepad_exit(void)
{
	  usb_deregister(&usb_gamepad_driver);
	   
}

module_init(usb_gamepad_init);
 module_exit(usb_gamepad_exit);

