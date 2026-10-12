package com.fortico.ronde_security.dtos.logo;
import java.time.OffsetDateTime;
public record LogoResponse(
    Long id,
    String photo,
    OffsetDateTime updateAt

) {
    
}
