#!/bin/bash

# NOTE: This script should be run as postgres user

echo "Attempt init db..."

echo "  Starting postgresql service..."
service postgresql start

db_name=test_db
# db_user=test_user
db_user=postgres
db_passwd=asd

# echo "  Assigning root password..."
# change root password to something so we can switch back to root later
# root_passwd=asd
# usermod --password $(echo $root_passwd | openssl passwd -1 -stdin) root

# echo "  Creating default database user: $db_user..."
# ret_create_user=$( psql -c "CREATE USER $db_user WITH PASSWORD '$db_passwd';" -U postgres 2>&1)
ret_alter_passwd=$( psql -c "ALTER USER $db_user WITH PASSWORD '$db_passwd';" -U postgres 2>&1)
if [[ $ret_alter_passwd == *"ERROR"* ]]
then
    echo "  Warning: Failed to set db password for user $db_user"
else
    echo "  Creating default database: $db_name..."
    ret_create_db=$(psql -c "CREATE DATABASE $db_name WITH OWNER $db_user;" -U postgres)
    if [[ $ret_create_db == *"ERROR"* && $ret_create_db == *"already exists"* ]]
    then
        echo "  Warning: Database $db_name already exists. Skipping db initialization"
    else
        ret_create_tables=$( psql -d test_db -U postgres -f data/init_db_tables.sql )
        if [[ $ret_create_tables == *"ERROR"* ]]
        then
            echo "  Failed to init database tables: $ret_create_tables"
        else
            echo "Database initialized successfully!"
        fi
    fi
fi
