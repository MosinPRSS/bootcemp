# notes

- `gorm.Model` is a struct:

```go
type Model struct {
 ID        uint `gorm:"primarykey"`
 CreatedAt time.Time
 UpdatedAt time.Time
 DeletedAt DeletedAt `gorm:"index"`
}
```
