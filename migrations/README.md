# Database Migrations

Each SQL file in this directory follows the naming convention:
`V{version}__{description}.sql`

For example:
- V1__initial_schema.sql
- V2__add_indices.sql

Migration files are executed in order based on their version numbers.
Once a migration is applied, it should never be modified.