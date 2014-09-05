mod-unique-id-decode
====================

Decode a string as created by apache httpd mod_unique_id

mod_unique_id_uudecoder.c
-------------------------

### compile

```
make mod_unique_id_uudecoder
```

### run

```
./mod_unique_id_uudecoder -i VAYyin8AAAEAAAKa@DMAAAAB
```


### test

```
./mod_unique_id_uudecoder -t
```

mod_unique_id_uudecoder.py
--------------------------

### run

```
python -m mod_unique_id_uudecoder -i VAYyin8AAAEAAAKa@DMAAAAB
```

mod_unique_id_uudecoder.go
--------------------------

### build

```
go build -o mod_unique_id_uudecoder_go mod_unique_id_uudecoder.go
```

### run

```
./mod_unique_id_uudecoder_go -i VAYyin8AAAEAAAKa@DMAAAAB
```

mod_unique_id_uudecoder.rb
--------------------------

### run

```
ruby mod_unique_id_uudecoder.rb -i VAYyin8AAAEAAAKa@DMAAAAB
```
