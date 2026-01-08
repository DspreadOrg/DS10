import json
import re
import sys 
import argparse
import os

class partition():

    def __init__(self,file_name,partition_name):
        self.file_name = file_name
        self.partition_name = partition_name
        self.json_name="PONY"
    def load(self):
        try:
            with open(self.file_name,'r') as jsonfile:
                jsondata=jsonfile.read()
                parsed_data = json.loads(jsondata)
                self.json_name = parsed_data["name"]
                #print("PPPP6",self.json_name)
                if(self.json_name!=self.partition_name):
                    self.json_name="PONY"
                    print("partition_name not fit")
                    return
                if(re.search("_02M", self.json_name , flags=0) or re.search("_2M", self.json_name , flags=0)):
                    self.flash_size = 0x200000
                if(re.search("_04M", self.json_name , flags=0) or re.search("_4M", self.json_name , flags=0)):
                    self.flash_size = 0x400000
                if(re.search("_08M", self.json_name , flags=0) or re.search("_8M", self.json_name , flags=0)):
                    self.flash_size = 0x800000
                flash_num=len(parsed_data["partitions"])
                i=1
                while(parsed_data["partitions"][flash_num-i]["name"] != "qspi"):
                    i += 1
                self.reserved_after = parsed_data["partitions"][flash_num-i]["partitions"]
                self.partition_num=len(parsed_data["partitions"][flash_num-i]["partitions"])
                print("partition_num:",self.partition_num)
                i=1
                while(self.reserved_after[self.partition_num-i]["name"] != "customer_app"):
                    i += 1
                    print(self.reserved_after[self.partition_num-i]["name"])
                    if(self.partition_num<i):
                        print("not fund partition:coustomer_app")
                if(self.partition_num>=i):
                    self.app_index=self.partition_num-i
                else:
                    self.app_index=0
                print("app_index:",self.app_index) 
        except FileNotFoundError:
            print("not found josn file")
        except json.JSONDecodeError:
            print("json parse error")
    

class kernel_map():
    def __init__(self,file_name):
        self.file_name = file_name
        
    def load(self):
        try:
            app_addr_str = ""
            with open(self.file_name,'r') as map_file:
                for line in map_file:
                    if 'Execution Region APP_CODE_IN_PSRAM' in line:
                        app_addr_str=line
                print("pppp1:",app_addr_str)        
                search=re.search(r'\(Base\:\s(0x[0-9a-fA-F]+)',app_addr_str)
                if search:
                    search_str=search.group(1)
                    self.app_ram_addr= int(search_str,16)
                    print("pppp:",search_str)


        except FileNotFoundError:
            print("not found kernelmap file")
 

class product():
    def __init__(self,file_name,product_name):
        self.file_name = file_name
        self.product_name = product_name

    def load(self):
        try:
             with open(self.file_name,'r') as jsonfile:
                jsondata=jsonfile.read()
                parsed_data = json.loads(jsondata)
                self.json_name = parsed_data["name"]
                product_num = len(parsed_data["variants"])
                print("ppp2:",product_num,self.product_name)
                i=1
                while(parsed_data["variants"][product_num-i]["name"] != self.product_name):
                    print("ppp3",parsed_data["variants"][product_num-i]["template"])
                    i+=1
                    if(product_num<i):
                        print("not found flash layout")
                        break
                if(product_num>=i):
                    self.layout_name = parsed_data["variants"][product_num-i]["layout"]
                else:
                    self.layout_name=""
                print(self.layout_name)
        except FileNotFoundError:
            print("not found josn file")
        except json.JSONDecodeError:
            print("json parse error")  

def str2size(sizestr):
    match = re.match(r'(\d+)([A-Za-z]+)', sizestr)
    if match:
        number_str = match.group(1)
        number = int(number_str)
        unit = match.group(2).lower()
        if unit == 'kib':
            number *= 1024
    else:
        print("No numbers and units matched")

    print("po",number)  
    return number
    

parser = argparse.ArgumentParser(description='Generate app start address')
parser.add_argument('--product','-p',type=str, default ="",required=True,help="a product's path")
parser.add_argument('--partition_dir','-d',type=str, default ="",required=True,help="a partition's path")
parser.add_argument('--partition','-a',type=str, default="",required=True,help="a partition's name")
parser.add_argument('--kernermap','-k',type=str, default='',required=True,help="kernel_map's path")
parser.add_argument('--ld','-l',type=str, default='',required=True,help="linkscript's path")
args = parser.parse_args()
print(args.partition_dir+"\\config\\product\\"+args.product,args.partition)
my_product = product(args.partition_dir+"\\config\\product\\"+args.product,args.partition)
my_product.load()
print(args.partition_dir+"\\config\\partition")
files = os.listdir(args.partition_dir+"\\config\\partition")
print(type(files))
print(files)
for i in files:
    path=os.path.join(args.partition_dir+"\\config\\partition\\"+i)
    #print(path,my_product.layout_name)
    my_partition = partition(path,my_product.layout_name)
    my_partition.load()
    if(my_partition.json_name!='PONY'):
        break
print(my_partition.file_name,my_partition.json_name)
my_kernel_map = kernel_map(args.kernermap)
my_kernel_map.load()
i=my_partition.partition_num-1
APPROSTARTADDR=0x80000000+my_partition.flash_size
PSIZE=0

while(i>=my_partition.app_index):
    if(my_partition.reserved_after[i]["name"] == "factory"):
        factory_num=len(my_partition.reserved_after[i]["partitions"])
        t=factory_num-1
        while(t>=0):
            text=my_partition.reserved_after[i]["partitions"][t]["size"]
            PSIZE += str2size(text)
            t -= 1
    else:
        text=my_partition.reserved_after[i]["size"]
        PSIZE += str2size(text)
    i -= 1

APPROSTARTADDR-=PSIZE
print("APPSTARTADDR:",hex(APPROSTARTADDR),hex(my_kernel_map.app_ram_addr))

try:
    with open(args.ld,"r") as ld_file:
        ld_data = ld_file.read()
        replacement1 = "ROM_START_ADDR = "+str(hex(APPROSTARTADDR))
        replacement2 = "RAM_START_ADDR = "+str(hex(my_kernel_map.app_ram_addr)) 
        print(replacement1,replacement2)
        pattern1 = re.compile(r'ROM_START_ADDR\s=\s*.*')
        pattern2 = re.compile(r'RAM_START_ADDR\s=\s*.*')
        replaced_contents = re.sub(pattern1,replacement1+";",ld_data)
        replaced_contents = re.sub(pattern2,replacement2+";",replaced_contents)
        with open(args.ld, "w") as file:
            file.write(replaced_contents)
except FileNotFoundError:
    print("not found ld file")